#pragma once

#include "domain/DeviceTypes.h"
#include "domain/User.h"

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
    void acknowledgeCurrentAlarm(const upkun::domain::User& user);
    QVector<QStringList> alarmRows(const QString& state, const QString& level, const QString& station, const QString& keyword, int limit = 100) const;
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
