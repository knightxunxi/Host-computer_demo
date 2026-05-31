#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace upkun::storage {

class AlarmRepository final {
public:
    bool openAlarm(int code, const QString& name, const QString& station, const QString& level, QString* errorMessage = nullptr);
    bool acknowledgeOpenAlarm(int code, const QString& userName, QString* errorMessage = nullptr);
    bool clearOpenAlarm(int code, QString* errorMessage = nullptr);
    QVector<QStringList> recentRows(int limit = 50) const;
};

} // namespace upkun::storage
