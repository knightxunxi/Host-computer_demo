#include "device/ModbusTcpClient.h"

#include "device/ModbusPointMap.h"

#include <algorithm>

namespace {

// Modbus TCP 使用大端序，Qt 的 QByteArray 按字节保存，这里集中处理 16 位值。
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

int communicationQuality(quint64 responses, quint64 timeouts, quint64 errors)
{
    const quint64 completed = responses + timeouts + errors;
    if (completed == 0) {
        return 100;
    }
    return static_cast<int>((responses * 100) / completed);
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

    m_timeoutTimer.setInterval(250);
    connect(&m_timeoutTimer, &QTimer::timeout, this, &ModbusTcpClient::checkTimeouts);
}

void ModbusTcpClient::connectToDevice(const upkun::domain::DeviceConnectionConfig& config)
{
    m_config = config;
    m_pollTimer.setInterval(m_config.statusPollMs);
    m_diagnostics = {};
    m_diagnostics.endpoint = QStringLiteral("%1:%2").arg(m_config.host).arg(m_config.port);
    updateConnectionState(upkun::domain::ConnectionState::Connecting);
    m_socket.abort();
    m_socket.connectToHost(m_config.host, m_config.port);
    m_timeoutTimer.start();
}

void ModbusTcpClient::disconnectFromDevice()
{
    m_reconnectTimer.stop();
    m_pollTimer.stop();
    m_timeoutTimer.stop();
    m_pending.clear();
    m_diagnostics.pendingRequests = 0;
    m_socket.disconnectFromHost();
    updateConnectionState(upkun::domain::ConnectionState::Disconnected);
}

void ModbusTcpClient::sendCommand(upkun::domain::DeviceCommand command)
{
    if (m_socket.state() != QAbstractSocket::ConnectedState) {
        emit commandFinished(command, false, QStringLiteral("PLC/模拟器未连接"));
        return;
    }

    // 业务层只认识“启动/停止”等命令；通信层负责把命令翻译成具体线圈地址。
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
    // 当前配方连续写入 40001-40010，后续多配方版本仍可复用这条下发通道。
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

void ModbusTcpClient::injectFault(int alarmCode)
{
    if (m_socket.state() != QAbstractSocket::ConnectedState) {
        emit commandFinished(upkun::domain::DeviceCommand::SimFault, false, QStringLiteral("PLC/模拟器未连接"));
        return;
    }

    sendWriteSingleRegister(
        static_cast<quint16>(modbus::toHoldingRegisterOffset(modbus::HoldingRegisters::SimFaultCode)),
        static_cast<quint16>(alarmCode));
    sendWriteSingleCoil(
        static_cast<quint16>(modbus::toCoilOffset(modbus::Coils::CmdSimFault)),
        true,
        upkun::domain::DeviceCommand::SimFault);
}

void ModbusTcpClient::poll()
{
    if (m_socket.state() != QAbstractSocket::ConnectedState) {
        return;
    }

    // 每轮先读传感器布尔量，再读寄存器状态；寄存器读完后发布完整快照。
    sendReadRequest(0x02, 0, 82, RequestKind::ReadDiscreteInputs);
    sendReadRequest(0x04, 0, 25, RequestKind::ReadInputRegisters);
}

void ModbusTcpClient::handleConnected()
{
    m_pending.clear();
    m_diagnostics.pendingRequests = 0;
    m_diagnostics.consecutiveTimeouts = 0;
    m_diagnostics.lastError.clear();
    m_buffer.clear();
    updateConnectionState(upkun::domain::ConnectionState::Connected);
    m_timeoutTimer.start();
    m_pollTimer.start();
    poll();
}

void ModbusTcpClient::handleDisconnected()
{
    m_pollTimer.stop();
    m_pending.clear();
    m_diagnostics.pendingRequests = 0;
    if (m_connectionState != upkun::domain::ConnectionState::Disconnected) {
        ++m_diagnostics.reconnectCount;
        updateConnectionState(upkun::domain::ConnectionState::Reconnecting);
        m_reconnectTimer.start(m_config.reconnectMs);
    } else {
        publishDiagnostics();
    }
}

void ModbusTcpClient::handleError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    recordError(m_socket.errorString());
    updateConnectionState(upkun::domain::ConnectionState::Error);
    emit errorOccurred({1, m_socket.errorString()});
}

void ModbusTcpClient::handleReadyRead()
{
    m_buffer.append(m_socket.readAll());

    while (m_buffer.size() >= 7) {
        // TCP 是流式协议，可能半包或粘包；按 MBAP 头的 length 字段切出完整 ADU。
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
    updateConnectionState(upkun::domain::ConnectionState::Reconnecting);
    m_socket.abort();
    m_socket.connectToHost(m_config.host, m_config.port);
}

void ModbusTcpClient::checkTimeouts()
{
    if (m_pending.isEmpty()) {
        return;
    }

    const QDateTime now = QDateTime::currentDateTime();
    QVector<quint16> timedOutTransactions;
    timedOutTransactions.reserve(m_pending.size());

    for (auto it = m_pending.cbegin(); it != m_pending.cend(); ++it) {
        if (it.value().sentAt.msecsTo(now) >= m_config.timeoutMs) {
            timedOutTransactions.append(it.key());
        }
    }

    if (timedOutTransactions.isEmpty()) {
        return;
    }

    for (const quint16 transactionId : timedOutTransactions) {
        const PendingRequest request = m_pending.take(transactionId);
        if (request.kind == RequestKind::WriteCommand) {
            emit commandFinished(request.command, false, QStringLiteral("Modbus 请求超时"));
        }
    }

    m_diagnostics.timeoutCount += static_cast<quint64>(timedOutTransactions.size());
    ++m_diagnostics.consecutiveTimeouts;
    m_diagnostics.pendingRequests = m_pending.size();
    m_diagnostics.lastError = QStringLiteral("Modbus 请求超时：%1 个请求未响应").arg(timedOutTransactions.size());
    m_diagnostics.qualityPercent = communicationQuality(
        m_diagnostics.totalResponses,
        m_diagnostics.timeoutCount,
        m_diagnostics.errorCount);
    publishDiagnostics();

    if (m_diagnostics.consecutiveTimeouts >= 2 && m_socket.state() == QAbstractSocket::ConnectedState) {
        m_pollTimer.stop();
        m_socket.abort();
    }
}

void ModbusTcpClient::sendReadRequest(quint8 function, quint16 address, quint16 quantity, RequestKind kind)
{
    QByteArray payload;
    appendU16(&payload, address);
    appendU16(&payload, quantity);
    PendingRequest request;
    request.kind = kind;
    request.command = upkun::domain::DeviceCommand::Start;
    sendAdu(function, payload, request);
}

void ModbusTcpClient::sendWriteSingleCoil(quint16 address, bool value, upkun::domain::DeviceCommand command)
{
    QByteArray payload;
    appendU16(&payload, address);
    appendU16(&payload, value ? 0xff00 : 0x0000);
    PendingRequest request;
    request.kind = RequestKind::WriteCommand;
    request.command = command;
    sendAdu(0x05, payload, request);
}

void ModbusTcpClient::sendWriteSingleRegister(quint16 address, quint16 value)
{
    QByteArray payload;
    appendU16(&payload, address);
    appendU16(&payload, value);
    PendingRequest request;
    request.kind = RequestKind::WriteRegister;
    request.command = upkun::domain::DeviceCommand::SimFault;
    sendAdu(0x06, payload, request);
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
    PendingRequest request;
    request.kind = RequestKind::WriteRecipe;
    request.command = upkun::domain::DeviceCommand::Start;
    sendAdu(0x10, payload, request);
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

    // transactionId 用来把异步响应匹配回当初的读写请求。
    request.sentAt = QDateTime::currentDateTime();
    m_pending.insert(transactionId, request);
    ++m_diagnostics.totalRequests;
    m_diagnostics.lastRequestAt = request.sentAt;
    m_diagnostics.pendingRequests = m_pending.size();
    m_diagnostics.qualityPercent = communicationQuality(
        m_diagnostics.totalResponses,
        m_diagnostics.timeoutCount,
        m_diagnostics.errorCount);
    publishDiagnostics();
    m_socket.write(adu);
}

void ModbusTcpClient::processResponse(const QByteArray& adu)
{
    if (adu.size() < 8) {
        return;
    }

    const quint16 transactionId = readU16(adu, 0);
    if (!m_pending.contains(transactionId)) {
        recordError(QStringLiteral("收到未知事务响应：%1").arg(transactionId));
        return;
    }

    const PendingRequest request = m_pending.take(transactionId);
    recordResponse(static_cast<int>(std::max<qint64>(0, request.sentAt.msecsTo(QDateTime::currentDateTime()))));
    const QByteArray pdu = adu.mid(7);
    const quint8 function = static_cast<quint8>(pdu.at(0));

    if ((function & 0x80) != 0) {
        recordError(QStringLiteral("Modbus 异常响应"));
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
    case RequestKind::WriteRegister:
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
    m_snapshot.stationInputs.feedingMaterialReady = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::FeedingMaterialReady));
    m_snapshot.stationInputs.conveyorRunning = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::ConveyorRunning));
    m_snapshot.stationInputs.bottleAtFilling = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::BottleAtFilling));
    m_snapshot.stationInputs.bottleAtCapping = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::BottleAtCapping));
    m_snapshot.stationInputs.bottleAtLabeling = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::BottleAtLabeling));
    m_snapshot.stationInputs.bottleAtOutfeed = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::BottleAtOutfeed));
    m_snapshot.stationInputs.fillingValveOk = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::FillingValveOk));
    m_snapshot.stationInputs.fillComplete = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::FillComplete));
    m_snapshot.stationInputs.capFeederReady = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::CapFeederReady));
    m_snapshot.stationInputs.capPresent = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::CapPresent));
    m_snapshot.stationInputs.cappingComplete = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::CappingComplete));
    m_snapshot.stationInputs.scaleReady = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::ScaleReady));
    m_snapshot.stationInputs.weightOk = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::WeightOk));
    m_snapshot.stationInputs.weightNg = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::WeightNg));
    m_snapshot.stationInputs.labelPrinterReady = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::LabelPrinterReady));
    m_snapshot.stationInputs.labelPaperOk = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::LabelPaperOk));
    m_snapshot.stationInputs.rejectCylinderHome = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::RejectCylinderHome));
    m_snapshot.stationInputs.rejectDetected = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::RejectDetected));
    m_snapshot.stationInputs.outfeedReady = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::OutfeedReady));
    m_snapshot.stationInputs.outfeedJam = bitAt(bits, modbus::toDiscreteInputOffset(modbus::DiscreteInputs::OutfeedJam));
}

void ModbusTcpClient::decodeInputRegisters(const QByteArray& pdu)
{
    if (pdu.size() < 2) {
        return;
    }

    const QByteArray registers = pdu.mid(2);
    // PDU 中寄存器是连续数组，点位表中的地址先转换为数组偏移再读取。
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
    m_snapshot.processValues.fillVolumeMl = reg(modbus::toInputRegisterOffset(modbus::InputRegisters::ActualFillVolume));
    m_snapshot.processValues.weightGram = reg(modbus::toInputRegisterOffset(modbus::InputRegisters::ActualWeight));
    m_snapshot.processValues.torqueCentinewtonMeter = reg(modbus::toInputRegisterOffset(modbus::InputRegisters::ActualTorque));
    m_snapshot.processValues.temperatureDeciCelsius = reg(modbus::toInputRegisterOffset(modbus::InputRegisters::Temperature));
    m_snapshot.processValues.pressureCentiMpa = reg(modbus::toInputRegisterOffset(modbus::InputRegisters::Pressure));
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
    m_diagnostics.state = state;
    if (m_diagnostics.endpoint.isEmpty()) {
        m_diagnostics.endpoint = QStringLiteral("%1:%2").arg(m_config.host).arg(m_config.port);
    }
    emit connectionChanged(state);
    publishDiagnostics();
}

void ModbusTcpClient::recordError(const QString& message)
{
    ++m_diagnostics.errorCount;
    m_diagnostics.lastError = message;
    m_diagnostics.qualityPercent = communicationQuality(
        m_diagnostics.totalResponses,
        m_diagnostics.timeoutCount,
        m_diagnostics.errorCount);
    publishDiagnostics();
}

void ModbusTcpClient::recordResponse(int roundTripMs)
{
    ++m_diagnostics.totalResponses;
    m_diagnostics.consecutiveTimeouts = 0;
    m_diagnostics.lastRoundTripMs = roundTripMs;
    m_diagnostics.lastResponseAt = QDateTime::currentDateTime();
    m_diagnostics.pendingRequests = m_pending.size();
    m_diagnostics.qualityPercent = communicationQuality(
        m_diagnostics.totalResponses,
        m_diagnostics.timeoutCount,
        m_diagnostics.errorCount);
    publishDiagnostics();
}

void ModbusTcpClient::publishDiagnostics()
{
    m_diagnostics.pendingRequests = m_pending.size();
    emit diagnosticsUpdated(m_diagnostics);
}

} // namespace upkun::device
