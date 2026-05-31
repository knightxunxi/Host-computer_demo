#pragma once

#include <QtGlobal>
#include <QString>

namespace upkun::domain {

enum class SystemState {
    Stopped = 0,
    Standby = 1,
    Running = 2,
    Paused = 3,
    Alarm = 4,
    EmergencyStop = 5,
    Resetting = 6
};

enum class RunMode {
    Unknown = 0,
    Manual = 1,
    Auto = 2,
    Maintenance = 3
};

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Reconnecting,
    Error
};

enum class Station {
    None = 0,
    Feeding = 1,
    Conveying = 2,
    Filling = 3,
    Capping = 4,
    Inspecting = 5,
    Labeling = 6,
    Rejecting = 7,
    Outfeeding = 8
};

enum class DeviceCommand {
    Start,
    Stop,
    Reset,
    AlarmAck,
    ModeAuto,
    ModeManual,
    BatchStart,
    BatchEnd,
    RejectTest,
    SimFault
};

struct ProductionCounters {
    int total = 0;
    int good = 0;
    int bad = 0;
    int batch = 0;
    int speed = 0;
};

struct ProcessValues {
    int fillVolumeMl = 0;
    int weightGram = 0;
    int torqueCentinewtonMeter = 0;
    int temperatureDeciCelsius = 0;
    int pressureCentiMpa = 0;
};

struct StationInputs {
    bool emergencyStopOk = true;
    bool safetyDoorOk = true;
    bool airPressureOk = true;
    bool plcReady = false;
};

struct DeviceSnapshot {
    SystemState systemState = SystemState::Standby;
    RunMode currentMode = RunMode::Auto;
    int currentAlarmCode = 0;
    Station activeStation = Station::None;
    ProductionCounters counters;
    ProcessValues processValues;
    StationInputs stationInputs;
};

struct DeviceConnectionConfig {
    QString host = QStringLiteral("127.0.0.1");
    quint16 port = 1502;
    int statusPollMs = 500;
    int processPollMs = 1000;
    int timeoutMs = 2000;
    int reconnectMs = 3000;
};

struct DeviceError {
    int code = 0;
    QString message;
};

} // namespace upkun::domain
