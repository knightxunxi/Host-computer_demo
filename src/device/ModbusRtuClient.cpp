#include "device/ModbusRtuClient.h"

#include "device/ModbusPointMap.h"
#include "device/ModbusRtuCodec.h"

namespace {

QString endpointText(const upkun::domain::DeviceConnectionConfig& config)
{
    return QStringLiteral("%1 %2bps slave %3").arg(config.serialPort).arg(config.baudRate).arg(config.slaveId);
}

QString transportStatusText()
{
#ifdef UPKUN_HAS_QT_SERIALPORT
    return QStringLiteral("当前构建已检测到 QtSerialPort，后续可在此客户端补真实串口收发和响应解析");
#else
    return QStringLiteral("当前构建未检测到 QtSerialPort，仅提供 Modbus RTU 帧编码学习");
#endif
}

} // namespace

namespace upkun::device {

ModbusRtuClient::ModbusRtuClient(QObject* parent)
    : IDeviceClient(parent)
{
}

void ModbusRtuClient::connectToDevice(const upkun::domain::DeviceConnectionConfig& config)
{
    m_config = config;
    m_diagnostics = {};
    m_diagnostics.endpoint = endpointText(config);
    m_diagnostics.state = upkun::domain::ConnectionState::Error;
    m_diagnostics.lastError = transportStatusText();
    ++m_diagnostics.errorCount;
    m_diagnostics.qualityPercent = 0;

    emit connectionChanged(upkun::domain::ConnectionState::Error);
    emit errorOccurred({2, m_diagnostics.lastError});
    publishDiagnostics();
}

void ModbusRtuClient::disconnectFromDevice()
{
    m_diagnostics.state = upkun::domain::ConnectionState::Disconnected;
    emit connectionChanged(upkun::domain::ConnectionState::Disconnected);
    publishDiagnostics();
}

void ModbusRtuClient::sendCommand(upkun::domain::DeviceCommand command)
{
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

    const auto frame = upkun::device::modbus_rtu::makeWriteSingleCoil(
        static_cast<quint8>(m_config.slaveId),
        static_cast<quint16>(address),
        true);
    Q_UNUSED(frame)
    reportUnavailable(QStringLiteral("RTU 命令帧已生成但未发送"));
    emit commandFinished(command, false, QStringLiteral("当前未启用串口传输"));
}

void ModbusRtuClient::writeRecipe(const upkun::domain::RecipeParameters& recipe)
{
    const auto frame = upkun::device::modbus_rtu::makeWriteSingleRegister(
        static_cast<quint8>(m_config.slaveId),
        static_cast<quint16>(upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::TargetSpeed)),
        static_cast<quint16>(recipe.targetSpeed));
    Q_UNUSED(frame)
    reportUnavailable(QStringLiteral("RTU 配方帧已生成但未发送"));
}

void ModbusRtuClient::publishDiagnostics()
{
    emit diagnosticsUpdated(m_diagnostics);
}

void ModbusRtuClient::reportUnavailable(const QString& action)
{
    ++m_diagnostics.totalRequests;
    ++m_diagnostics.errorCount;
    m_diagnostics.lastRequestAt = QDateTime::currentDateTime();
    m_diagnostics.lastError = action + QStringLiteral("：%1").arg(transportStatusText());
    m_diagnostics.qualityPercent = 0;
    publishDiagnostics();
}

} // namespace upkun::device
