#pragma once

#include <QDateTime>
#include <QString>

namespace upkun::domain {

enum class AlarmState {
    ActiveUnacked,
    ActiveAcked,
    ClearedUnacked,
    Closed
};

enum class AlarmLevel {
    Info,
    Warning,
    Error,
    Critical
};

struct AlarmRecord {
    int code = 0;
    QString name;
    QString station;
    AlarmLevel level = AlarmLevel::Warning;
    AlarmState state = AlarmState::Closed;
    QDateTime triggeredAt;
    QDateTime ackedAt;
    QDateTime clearedAt;
    QString ackedBy;
};

} // namespace upkun::domain
