#include "simulator/LineSimulator.h"

#include "device/ModbusPointMap.h"
#include "domain/DeviceTypes.h"

#include <algorithm>

namespace {

constexpr int kPointCapacity = 128;

int clampPercent(int value)
{
    return std::clamp(value, 0, 100);
}

} // namespace

namespace upkun::simulator {

LineSimulator::LineSimulator(QObject* parent)
    : QObject(parent)
    , m_coils(kPointCapacity)
    , m_discreteInputs(kPointCapacity)
    , m_holdingRegisters(kPointCapacity)
    , m_inputRegisters(kPointCapacity)
{
    resetData();
    connect(&m_cycleTimer, &QTimer::timeout, this, &LineSimulator::advanceCycle);
    m_cycleTimer.start(1000);
}

bool LineSimulator::readCoil(int offset) const
{
    return offset >= 0 && offset < m_coils.size() ? m_coils.at(offset) : false;
}

bool LineSimulator::readDiscreteInput(int offset) const
{
    return offset >= 0 && offset < m_discreteInputs.size() ? m_discreteInputs.at(offset) : false;
}

quint16 LineSimulator::readHoldingRegister(int offset) const
{
    return offset >= 0 && offset < m_holdingRegisters.size() ? m_holdingRegisters.at(offset) : 0;
}

quint16 LineSimulator::readInputRegister(int offset) const
{
    return offset >= 0 && offset < m_inputRegisters.size() ? m_inputRegisters.at(offset) : 0;
}

bool LineSimulator::writeCoil(int offset, bool value)
{
    if (offset < 0 || offset >= m_coils.size()) {
        return false;
    }

    m_coils[offset] = value;
    if (value) {
        // 上位机写线圈在本项目中表示“按钮脉冲”，模拟器处理后立即清零。
        handleCommand(offset);
        m_coils[offset] = false;
    }

    updateInputs();
    updateInputRegisters();
    emit stateChanged();
    return true;
}

bool LineSimulator::writeHoldingRegister(int offset, quint16 value)
{
    if (offset < 0 || offset >= m_holdingRegisters.size()) {
        return false;
    }

    m_holdingRegisters[offset] = value;
    updateInputRegisters();
    emit stateChanged();
    return true;
}

bool LineSimulator::writeHoldingRegisters(int offset, const QVector<quint16>& values)
{
    if (offset < 0 || offset + values.size() > m_holdingRegisters.size()) {
        return false;
    }

    for (qsizetype i = 0; i < values.size(); ++i) {
        m_holdingRegisters[offset + i] = values.at(i);
    }

    updateInputRegisters();
    emit stateChanged();
    return true;
}

void LineSimulator::triggerAlarm(int alarmCode)
{
    m_alarmCode = alarmCode;
    m_running = false;
    m_activeStation = 0;
    updateInputs();
    updateInputRegisters();
    emit stateChanged();
}

void LineSimulator::clearAlarm()
{
    m_alarmCode = 0;
    m_running = false;
    m_activeStation = 0;
    updateInputs();
    updateInputRegisters();
    emit stateChanged();
}

void LineSimulator::advanceCycle()
{
    if (!m_running || m_alarmCode != 0) {
        updateInputs();
        updateInputRegisters();
        emit stateChanged();
        return;
    }

    // 学习版用每秒切换一个工位来模拟全自动包装线的节拍。
    m_activeStation = m_activeStation % 8 + 1;

    if (m_activeStation == static_cast<int>(upkun::domain::Station::Outfeeding)) {
        // 产品到达下料工位时才计一次产量；质量率来自配方中的模拟参数。
        const int qualityRate = clampPercent(holdingValue(
            upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::SimQualityRate),
            98));
        const bool good = (m_totalCount % 100) < qualityRate;
        ++m_totalCount;
        ++m_batchCount;
        if (good) {
            ++m_goodCount;
        } else {
            ++m_badCount;
        }
    }

    updateInputs();
    updateInputRegisters();
    emit stateChanged();
}

void LineSimulator::resetData()
{
    std::fill(m_coils.begin(), m_coils.end(), false);
    std::fill(m_discreteInputs.begin(), m_discreteInputs.end(), false);
    std::fill(m_holdingRegisters.begin(), m_holdingRegisters.end(), 0);
    std::fill(m_inputRegisters.begin(), m_inputRegisters.end(), 0);

    // 默认保持寄存器相当于 PLC 里的初始工艺参数，配方下发会覆盖这些值。
    m_holdingRegisters[upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::TargetSpeed)] = 60;
    m_holdingRegisters[upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::FillVolume)] = 500;
    m_holdingRegisters[upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::FillTime)] = 1000;
    m_holdingRegisters[upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::CappingTorque)] = 125;
    m_holdingRegisters[upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::WeightMin)] = 480;
    m_holdingRegisters[upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::WeightMax)] = 520;
    m_holdingRegisters[upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::LabelMode)] = 1;
    m_holdingRegisters[upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::BatchTargetCount)] = 1000;
    m_holdingRegisters[upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::AlarmDelay)] = 500;
    m_holdingRegisters[upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::SimQualityRate)] = 98;

    updateInputs();
    updateInputRegisters();
}

void LineSimulator::handleCommand(int offset)
{
    using upkun::device::modbus::Coils;
    using upkun::device::modbus::toCoilOffset;

    if (offset == toCoilOffset(Coils::CmdStart)) {
        if (m_alarmCode == 0) {
            m_running = true;
            if (m_activeStation == 0) {
                m_activeStation = 1;
            }
        }
    } else if (offset == toCoilOffset(Coils::CmdStop)) {
        m_running = false;
        m_activeStation = 0;
    } else if (offset == toCoilOffset(Coils::CmdReset)) {
        m_alarmCode = 0;
        m_running = false;
        m_activeStation = 0;
    } else if (offset == toCoilOffset(Coils::CmdModeAuto)) {
        m_inputRegisters[upkun::device::modbus::toInputRegisterOffset(upkun::device::modbus::InputRegisters::CurrentMode)] = static_cast<quint16>(upkun::domain::RunMode::Auto);
    } else if (offset == toCoilOffset(Coils::CmdModeManual)) {
        m_inputRegisters[upkun::device::modbus::toInputRegisterOffset(upkun::device::modbus::InputRegisters::CurrentMode)] = static_cast<quint16>(upkun::domain::RunMode::Manual);
    } else if (offset == toCoilOffset(Coils::CmdBatchStart)) {
        m_batchCount = 0;
    } else if (offset == toCoilOffset(Coils::CmdBatchEnd)) {
        m_batchCount = 0;
    } else if (offset == toCoilOffset(Coils::CmdSimFault)) {
        triggerAlarm(5001);
    }
}

void LineSimulator::updateInputs()
{
    using upkun::device::modbus::DiscreteInputs;
    using upkun::device::modbus::toDiscreteInputOffset;

    std::fill(m_discreteInputs.begin(), m_discreteInputs.end(), false);
    // 公共安全联锁默认正常；报警注入主要通过具体工位传感器改变状态。
    m_discreteInputs[toDiscreteInputOffset(DiscreteInputs::EstopOk)] = true;
    m_discreteInputs[toDiscreteInputOffset(DiscreteInputs::SafetyDoorOk)] = true;
    m_discreteInputs[toDiscreteInputOffset(DiscreteInputs::AirPressureOk)] = true;
    m_discreteInputs[toDiscreteInputOffset(DiscreteInputs::PowerOk)] = true;
    m_discreteInputs[toDiscreteInputOffset(DiscreteInputs::PlcReady)] = true;

    // 以下偏移暂按点位表初版直接映射，后续 M13 会继续命名化和细化传感器。
    m_discreteInputs[10] = true;
    m_discreteInputs[20] = m_running;
    m_discreteInputs[21] = m_activeStation == 3;
    m_discreteInputs[22] = m_activeStation == 4;
    m_discreteInputs[23] = m_activeStation == 6;
    m_discreteInputs[24] = m_activeStation == 8;
    m_discreteInputs[30] = m_alarmCode != 4001;
    m_discreteInputs[31] = m_activeStation != 3;
    m_discreteInputs[32] = m_activeStation == 3;
    m_discreteInputs[40] = m_alarmCode != 5001;
    m_discreteInputs[41] = m_activeStation != 4;
    m_discreteInputs[42] = m_activeStation == 4;
    m_discreteInputs[50] = true;
    m_discreteInputs[51] = m_alarmCode == 0;
    m_discreteInputs[52] = m_alarmCode == 6002;
    m_discreteInputs[60] = m_alarmCode != 7001;
    m_discreteInputs[61] = m_alarmCode != 7002;
    m_discreteInputs[70] = m_activeStation != 7;
    m_discreteInputs[71] = m_activeStation == 7;
    m_discreteInputs[80] = m_alarmCode == 9001;
    m_discreteInputs[81] = true;
}

void LineSimulator::updateInputRegisters()
{
    using upkun::device::modbus::InputRegisters;
    using upkun::device::modbus::toInputRegisterOffset;

    // 输入寄存器是上位机主监控页的数据来源，集中反映当前模拟产线快照。
    const auto systemState = m_alarmCode != 0
        ? upkun::domain::SystemState::Alarm
        : (m_running ? upkun::domain::SystemState::Running : upkun::domain::SystemState::Standby);

    m_inputRegisters[toInputRegisterOffset(InputRegisters::SystemState)] = static_cast<quint16>(systemState);
    if (m_inputRegisters[toInputRegisterOffset(InputRegisters::CurrentMode)] == 0) {
        m_inputRegisters[toInputRegisterOffset(InputRegisters::CurrentMode)] = static_cast<quint16>(upkun::domain::RunMode::Auto);
    }
    m_inputRegisters[toInputRegisterOffset(InputRegisters::CurrentAlarmCode)] = static_cast<quint16>(m_alarmCode);
    m_inputRegisters[toInputRegisterOffset(InputRegisters::CommState)] = 1;
    m_inputRegisters[toInputRegisterOffset(InputRegisters::ActiveStation)] = static_cast<quint16>(m_activeStation);
    m_inputRegisters[toInputRegisterOffset(InputRegisters::TotalCount)] = static_cast<quint16>(m_totalCount);
    m_inputRegisters[toInputRegisterOffset(InputRegisters::GoodCount)] = static_cast<quint16>(m_goodCount);
    m_inputRegisters[toInputRegisterOffset(InputRegisters::BadCount)] = static_cast<quint16>(m_badCount);
    m_inputRegisters[toInputRegisterOffset(InputRegisters::BatchCount)] = static_cast<quint16>(m_batchCount);
    m_inputRegisters[toInputRegisterOffset(InputRegisters::CurrentSpeed)] = static_cast<quint16>(m_running ? holdingValue(upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::TargetSpeed), 60) : 0);

    const int fillVolume = holdingValue(upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::FillVolume), 500);
    const int torque = holdingValue(upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::CappingTorque), 125);
    m_inputRegisters[toInputRegisterOffset(30021)] = static_cast<quint16>(m_activeStation >= 3 ? fillVolume : 0);
    m_inputRegisters[toInputRegisterOffset(30022)] = static_cast<quint16>(m_activeStation >= 5 ? fillVolume : 0);
    m_inputRegisters[toInputRegisterOffset(30023)] = static_cast<quint16>(m_activeStation >= 4 ? torque : 0);
    m_inputRegisters[toInputRegisterOffset(30024)] = 245;
    m_inputRegisters[toInputRegisterOffset(30025)] = 62;
}

int LineSimulator::holdingValue(int offset, int fallback) const
{
    return offset >= 0 && offset < m_holdingRegisters.size() ? static_cast<int>(m_holdingRegisters.at(offset)) : fallback;
}

} // namespace upkun::simulator
