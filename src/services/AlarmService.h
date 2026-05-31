#pragma once

#include "domain/DeviceTypes.h"

#include <QObject>
#include <QStringList>
#include <QVector>

namespace upkun::storage {
class AlarmRepository;
class OperationLogRepository;
}

namespace upkun::services {

class AlarmService final : public QObject {
    Q_OBJECT

public:
    AlarmService(upkun::storage::AlarmRepository* alarmRepository,
        upkun::storage::OperationLogRepository* operationLogRepository,
        QObject* parent = nullptr);

    void processSnapshot(const upkun::domain::DeviceSnapshot& snapshot);
    void acknowledgeCurrentAlarm(const QString& userName);
    QVector<QStringList> recentAlarmRows(int limit = 50) const;
    QVector<QStringList> recentOperationRows(int limit = 50) const;

signals:
    void recordsChanged();

private:
    QString alarmName(int code) const;
    QString alarmStation(int code) const;
    QString alarmLevel(int code) const;

    upkun::storage::AlarmRepository* m_alarmRepository = nullptr;
    upkun::storage::OperationLogRepository* m_operationLogRepository = nullptr;
    int m_currentAlarmCode = 0;
};

} // namespace upkun::services
