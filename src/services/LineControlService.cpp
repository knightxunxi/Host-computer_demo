#include "services/LineControlService.h"

namespace upkun::services {

LineControlService::LineControlService(QObject* parent)
    : QObject(parent)
{
}

bool LineControlService::canStart(const upkun::domain::DeviceSnapshot& snapshot, QString* reason) const
{
    if (!snapshot.stationInputs.plcReady) {
        if (reason != nullptr) {
            *reason = QStringLiteral("PLC/模拟器未就绪");
        }
        return false;
    }

    if (!snapshot.stationInputs.emergencyStopOk) {
        if (reason != nullptr) {
            *reason = QStringLiteral("急停未恢复");
        }
        return false;
    }

    if (!snapshot.stationInputs.safetyDoorOk) {
        if (reason != nullptr) {
            *reason = QStringLiteral("安全门未关闭");
        }
        return false;
    }

    if (!snapshot.stationInputs.airPressureOk) {
        if (reason != nullptr) {
            *reason = QStringLiteral("气压不足");
        }
        return false;
    }

    if (snapshot.currentAlarmCode != 0) {
        if (reason != nullptr) {
            *reason = QStringLiteral("当前存在报警");
        }
        return false;
    }

    if (snapshot.currentMode != upkun::domain::RunMode::Auto) {
        if (reason != nullptr) {
            *reason = QStringLiteral("当前不是自动模式");
        }
        return false;
    }

    return true;
}

} // namespace upkun::services
