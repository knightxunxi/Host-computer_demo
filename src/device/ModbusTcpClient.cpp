#include "device/ModbusTcpClient.h"

#include "device/ModbusPointMap.h"

namespace {

quint16 readU16(const QByteArray& data, int offset)
{
    const auto high = static_cast<quint8>(data.at(offset));
    const auto low = static_cast<quint8>(data.at(offset + 1));
    return static_cast<quint16>((high << 8) | low);
}

void appendU16(QByteArray* data, quint16 value)
{
    data->append(static_cast<char>((value >> 8) & 0xff));
    data->append(static_cast<char>(value & 0xff));
}

bool bitAt(const QByteArray& data, int bitIndex)
{
    const int byteIndex = bitIndex / 8;
    const int offset = bitIndex % 8;
    if (byteIndex < 0 || byteIndex >= data.size()) {
        return false;
    }
    return (static_cast<quint8>(data.at(byteIndex)) & (1 << offset)) != 0;
}

} // namespace

namespace upkun::device {

ModbusTcpClient::ModbusTcpClient(QObject* parent)
    : IDeviceClient(parent)
{
    connect(&m_socket, &QTcpSocket::connected, this, &ModbusTcpClient::handleConnected);
    connect(&m_socket, &QTcpSocket::disconnected, this, &ModbusTcpClient::handleDisconnected);
    connect(&m_socket, &QTcpSocket::readyRead, this, &ModbusTcpClient::handleReadyRead);
    connect(&m_socket, &QTcpSocket::errorOccurred, this, &ModbusTcpClient::handleError);

    m_pollTimer.setInterval(m_config.statusPollMs);
    connect(&m_pollTimer, &QTimer::timeout, this, &ModbusTcpClient::poll);

    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &ModbusTcpClient::reconnect);
}

void ModbusTcpClient::connectToDevice(const upkun::domain::DeviceConnectionConfig& config)
{
    m_config = config;
    m_pollTimer.setInterval(m_config.statusPollMs);
    updateConnectionState(upkun::domain::ConnectionState::Connecting);
    m_socket.abort();
    m_socket.connectToHost(m_config.host, m_config.port);
}

void ModbusTcpClient::disconnectFromDevice()
{
    m_reconnectTimer.stop();
    m_pollTimer.stop();
    m_pending.clear();
    m_socket.disconnectFromHost();
    updateConnectionState(upkun::domain::ConnectionState::Disconnected);
}

void ModbusTcpClient::sendCommand(upkun::domain::DeviceCommand command)
{
    if (m_socket.state() != QAbstractSocket::ConnectedState) {
        emit commandFinished(command, false, QStringLiteral("PLC/模拟器未连接"));
        return;
    }

    using modbus::Coils;
    using modbus::toCoilOffset;

    int address = -1;
    switch (command) {
    case upkun::domain::DeviceCommand::Start:
        address = toCoilOffset(Coils::CmdStart);
        break;
    case upkun::domain::DeviceCommand::Stop:
        address = toCoilOffset(Coils::CmdStop);
        break;
    case upkun::domain::DeviceCommand::Reset:
        address = toCoilOffset(Coils::CmdReset);
        break;
    case upkun::domain::DeviceCommand::AlarmAck:
        address = toCoilOffset(Coils::CmdAlarmAck);
        break;
    case upkun::domain::DeviceCommand::ModeAuto:
        address = toCoilOffset(Coils::CmdModeAuto);
        break;
    case upkun::domain::DeviceCommand::ModeManual:
        address = toCoilOffset(Coils::CmdModeManual);
        break;
    case upkun::domain::DeviceCommand::BatchStart:
        address = toCoilOffset(Coils::CmdBatchStart);
        break;
    case upkun::domain::DeviceCommand::BatchEnd:
        address = toCoilOffset(Coils::CmdBatchEnd);
        break;
    case upkun::domain::DeviceCommand::RejectTest:
        address = toCoilOffset(Coils::CmdRejectTest);
        break;
    case upkun::domain::DeviceCommand::SimFault:
        address = toCoilOffset(Coils::CmdSimFault);
        break;
    }

    sendWriteSingleCoil(static_cast<quint16>(address), true, command);
}

void ModbusTcpClient::writeRecipe(const upkun::domain::RecipeParameters& recipe)
{
    QVector<quint16> values {
        static_cast<quint16>(recipe.targetSpeed),
        static_cast<quint16>(recipe.fillVolumeMl),
        static_cast<quint16>(recipe.fillTimeMs),
        static_cast<quint16>(recipe.cappingTorqueCentinewtonMeter),
        static_cast<quint16>(recipe.weightMinGram),
        static_cast<quint16>(recipe.weightMaxGram),
        static_cast<quint16>(recipe.labelMode),
        static_cast<quint16>(recipe.batchTargetCount),
        500,
        static_cast<quint16>(recipe.simulationQualityRate),
    };
    sendWriteMultipleRegisters(0, values);
}

void ModbusTcpClient::poll()
{
    if (m_socket.state() != QAbstractSocket::ConnectedState) {
        return;
    }

    sendReadRequest(0x02, 0, 82, RequestKind::ReadDiscreteInputs);
    sendReadRequest(0x04, 0, 25, RequestKind::ReadInputRegisters);
}

void ModbusTcpClient::handleConnected()
{
    m_pending.clear();
    m_buffer.clear();
    updateConnectionState(upkun::domain::ConnectionState::Connected);
    m_pollTimer.start();
    poll();
}

void ModbusTcpClient::handleDisconnected()
{
    m_pollTimer.stop();
    if (m_connectionState != upkun::domain::ConnectionState::Disconnected) {
        updateConnectionState(upkun::domain::ConnectionState::Reconnecting);
        m_reconnectTimer.start(m_config.reconnectMs);
    }
}

void ModbusTcpClient::handleError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    emit errorOccurred({1, m_socket.errorString()});
}

void ModbusTcpClient::handleReadyRead()
{
    m_buffer.append(m_socket.readAll());

    while (m_buffer.size() >= 7) {
        const quint16 length = readU16(m_buffer, 4);
        const int aduSize = 6 + length;
        if (m_buffer.size() < aduSize) {
            break;
        }

        const QByteArray response = m_buffer.left(aduSize);
        m_buffer.remove(0, aduSize);
        processResponse(response);
    }
}

void ModbusTcpClient::reconnect()
{
    if (m_connectionState == upkun::domain::ConnectionState::Disconnected) {
        return;
    }
    m_socket.abort();
    m_socket.connectToHost(m_config.host, m_config.port);
}

void ModbusTcpClient::sendReadRequest(quint8 function, quint16 address, quint16 quantity, RequestKind kind)
{
    QByteArray payload;
    appendU16(&payload, address);
    appendU16(&payload, quantity);
    sendAdu(function, payload, {kind, upkun::domain::DeviceCommand::Start});
}

void ModbusTcpClient::sendWriteSingleCoil(quint16 address, bool value, upkun::domain::DeviceCommand command)
{
    QByteArray payload;
    appendU16(&payload, address);
    appendU16(&payload, value ? 0xff00 : 0x0000);
    sendAdu(0x05, payload, {RequestKind::WriteCommand, command});
}

void ModbusTcpClient::sendWriteMultipleRegisters(quint16 address, const QVector<quint16>& values)
{
    QByteArray payload;
    appendU16(&payload, address);
    appendU16(&payload, static_cast<quint16>(values.size()));
    payload.append(static_cast<char>(values.size() * 2));
    for (const quint16 value : values) {
        appendU16(&payload, value);
    }
    sendAdu(0x10, payload, {RequestKind::WriteRecipe, upkun::domain::DeviceCommand::Start});
}

void ModbusTcpClient::sendAdu(quint8 function, const QByteArray& payload, PendingRequest request)
{
    const quint16 transactionId = nextTransactionId();
    QByteArray adu;
    appendU16(&adu, transactionId);
    appendU16(&adu, 0);
    appendU16(&adu, static_cast<quint16>(payload.size() + 2));
    adu.append(static_cast<char>(1));
    adu.append(static_cast<char>(function));
    adu.append(payload);

    m_pending.insert(transactionId, request);
    m_socket.write(adu);
}

void ModbusTcpClient::processResponse(const QByteArray& adu)
{
    if (adu.size() < 8) {
        return;
    }

    const quint16 transactionId = readU16(adu, 0);
    const PendingRequest request = m_pending.take(transactionId);
    const QByteArray pdu = adu.mid(7);
    const quint8 function = static_cast<quint8>(pdu.at(0));

    if ((function & 0x80) != 0) {
        if (request.kind == RequestKind::WriteCommand) {
            emit commandFinished(request.command, false, QStringLiteral("Modbus 异常响应"));
        }
        return;
    }

    switch (request.kind) {
    case RequestKind::ReadDiscreteInputs:
        decodeDiscreteInputs(pdu);
        break;
    case RequestKind::ReadInputRegisters:
        decodeInputRegisters(pdu);
        emit snapshotUpdated(m_snapshot);
        break;
    case RequestKind::WriteCommand:
        emit commandFinished(request.command, true, QStringLiteral("命令已下发"));
        poll();
        break;
    case RequestKind::WriteRecipe:
        poll();
        break;
    }
}

void ModbusTcpClient::decodeDiscreteInputs(const QByteArray& pdu)
{
    if (pdu.size() < 2) {
        return;
    }

    const QByteArray bits = pdu.mid(2);
    m_snapshot.stationInputs.emergencyStopOk = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::EstopOk));
    m_snapshot.stationInputs.safetyDoorOk = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::SafetyDoorOk));
    m_snapshot.stationInputs.airPressureOk = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::AirPressureOk));
    m_snapshot.stationInputs.plcReady = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::PlcReady));
}

void ModbusTcpClient::decodeInputRegisters(const QByteArray& pdu)
{
    if (pdu.size() < 2) {
        return;
    }

    const QByteArray registers = pdu.mid(2);
    auto reg = [&registers](int offset) -> quint16 {
        const int byteOffset = offset * 2;
        return byteOffset + 1 < registers.size() ? readU16(registers, byteOffset) : 0;
    };

    m_snapshot.systemState = static_cast<upkun::domain::SystemState>(reg(modbus::toInputRegisterOffset(modbus::InputRegisters::SystemState)));
    m_snapshot.currentMode = static_cast<upkun::domain::RunMode>(reg(modbus::toInputRegisterOffset(modbus::InputRegisters::CurrentMode)));
    m_snapshot.currentAlarmCode = reg(modbus::toInputRegisterOffset(modbus::InputRegisters::CurrentAlarmCode));
    m_snapshot.activeStation = static_cast<upkun::domain::Station>(reg(modbus::toInputRegisterOffset(modbus::InputRegisters::ActiveStation)));
    m_snapshot.counters.total = reg(modbus::toInputRegisterOffset(modbus::InputRegisters::TotalCount));
    m_snapshot.counters.good = reg(modbus::toInputRegisterOffset(modbus::InputRegisters::GoodCount));
    m_snapshot.counters.bad = reg(modbus::toInputRegisterOffset(modbus::InputRegisters::BadCount));
    m_snapshot.counters.batch = reg(modbus::toInputRegisterOffset(modbus::InputRegisters::BatchCount));
    m_snapshot.counters.speed = reg(modbus::toInputRegisterOffset(modbus::InputRegisters::CurrentSpeed));
    m_snapshot.processValues.fillVolumeMl = reg(modbus::toInputRegisterOffset(30021));
    m_snapshot.processValues.weightGram = reg(modbus::toInputRegisterOffset(30022));
    m_snapshot.processValues.torqueCentinewtonMeter = reg(modbus::toInputRegisterOffset(30023));
    m_snapshot.processValues.temperatureDeciCelsius = reg(modbus::toInputRegisterOffset(30024));
    m_snapshot.processValues.pressureCentiMpa = reg(modbus::toInputRegisterOffset(30025));
}

quint16 ModbusTcpClient::nextTransactionId()
{
    ++m_transactionId;
    if (m_transactionId == 0) {
        ++m_transactionId;
    }
    return m_transactionId;
}

void ModbusTcpClient::updateConnectionState(upkun::domain::ConnectionState state)
{
    if (m_connectionState == state) {
        return;
    }
    m_connectionState = state;
    emit connectionChanged(state);
}

} // namespace upkun::device
