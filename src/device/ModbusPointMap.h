#pragma once

namespace upkun::device::modbus {

// Modbus 文档通常使用 00001/10001/30001/40001 这类人工地址；
// 报文里实际发送的是从 0 开始的偏移量，所以统一在这里做转换。
struct Bases {
    static constexpr int Coil = 1;
    static constexpr int DiscreteInput = 10001;
    static constexpr int InputRegister = 30001;
    static constexpr int HoldingRegister = 40001;
};

constexpr int toCoilOffset(int address)
{
    return address - Bases::Coil;
}

constexpr int toDiscreteInputOffset(int address)
{
    return address - Bases::DiscreteInput;
}

constexpr int toInputRegisterOffset(int address)
{
    return address - Bases::InputRegister;
}

constexpr int toHoldingRegisterOffset(int address)
{
    return address - Bases::HoldingRegister;
}

struct Coils {
    // 线圈用于上位机向 PLC 写入瞬时命令，模拟器收到 true 后会立即复位。
    static constexpr int CmdStart = 1;
    static constexpr int CmdStop = 2;
    static constexpr int CmdReset = 3;
    static constexpr int CmdAlarmAck = 4;
    static constexpr int CmdModeAuto = 5;
    static constexpr int CmdModeManual = 6;
    static constexpr int CmdBatchStart = 7;
    static constexpr int CmdBatchEnd = 8;
    static constexpr int CmdRejectTest = 9;
    static constexpr int CmdSimFault = 10;
};

struct DiscreteInputs {
    // 离散输入表示传感器/联锁这类布尔状态，只读。
    static constexpr int EstopOk = 10001;
    static constexpr int SafetyDoorOk = 10002;
    static constexpr int AirPressureOk = 10003;
    static constexpr int PowerOk = 10004;
    static constexpr int PlcReady = 10005;
};

struct InputRegisters {
    // 输入寄存器表示 PLC 汇总后的实时状态和生产数据，只读。
    static constexpr int SystemState = 30001;
    static constexpr int CurrentMode = 30002;
    static constexpr int CurrentAlarmCode = 30003;
    static constexpr int CommState = 30004;
    static constexpr int ActiveStation = 30005;
    static constexpr int TotalCount = 30011;
    static constexpr int GoodCount = 30012;
    static constexpr int BadCount = 30013;
    static constexpr int BatchCount = 30014;
    static constexpr int CurrentSpeed = 30015;
};

struct HoldingRegisters {
    // 保持寄存器用于上位机下发可调整参数，对应当前学习版配方。
    static constexpr int TargetSpeed = 40001;
    static constexpr int FillVolume = 40002;
    static constexpr int FillTime = 40003;
    static constexpr int CappingTorque = 40004;
    static constexpr int WeightMin = 40005;
    static constexpr int WeightMax = 40006;
    static constexpr int LabelMode = 40007;
    static constexpr int BatchTargetCount = 40008;
    static constexpr int AlarmDelay = 40009;
    static constexpr int SimQualityRate = 40010;
};

} // namespace upkun::device::modbus
