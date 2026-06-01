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

    static constexpr int FeedingMaterialReady = 10011;
    static constexpr int ConveyorRunning = 10021;
    static constexpr int BottleAtFilling = 10022;
    static constexpr int BottleAtCapping = 10023;
    static constexpr int BottleAtLabeling = 10024;
    static constexpr int BottleAtOutfeed = 10025;
    static constexpr int FillingValveOk = 10031;
    static constexpr int FillNozzleDown = 10032;
    static constexpr int FillComplete = 10033;
    static constexpr int CapFeederReady = 10041;
    static constexpr int CapPresent = 10042;
    static constexpr int CappingComplete = 10043;
    static constexpr int ScaleReady = 10051;
    static constexpr int WeightOk = 10052;
    static constexpr int WeightNg = 10053;
    static constexpr int LabelPrinterReady = 10061;
    static constexpr int LabelPaperOk = 10062;
    static constexpr int RejectCylinderHome = 10071;
    static constexpr int RejectDetected = 10072;
    static constexpr int OutfeedJam = 10081;
    static constexpr int OutfeedReady = 10082;
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
    static constexpr int ActualFillVolume = 30021;
    static constexpr int ActualWeight = 30022;
    static constexpr int ActualTorque = 30023;
    static constexpr int Temperature = 30024;
    static constexpr int Pressure = 30025;
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
    static constexpr int SimFaultCode = 40011;
};

} // namespace upkun::device::modbus
