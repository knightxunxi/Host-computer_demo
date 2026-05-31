#pragma once

#include "domain/User.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace upkun::storage {

class OperationLogRepository final {
public:
    bool append(const QString& action, const QString& target, const QString& result, const QString& message, QString* errorMessage = nullptr);
    bool append(const upkun::domain::User& user, const QString& action, const QString& target, const QString& result, const QString& message, QString* errorMessage = nullptr);
    QVector<QStringList> recentRows(int limit = 50) const;
};

} // namespace upkun::storage
