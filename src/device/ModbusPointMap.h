#pragma once

namespace upkun::device::modbus {

struct Coils {
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
    static constexpr int EstopOk = 10001;
    static constexpr int SafetyDoorOk = 10002;
    static constexpr int AirPressureOk = 10003;
    static constexpr int PowerOk = 10004;
    static constexpr int PlcReady = 10005;
};

struct InputRegisters {
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
