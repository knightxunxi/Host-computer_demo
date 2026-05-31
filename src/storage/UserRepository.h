#pragma once

#include "domain/User.h"

#include <QString>
#include <QVector>
#include <optional>

namespace upkun::storage {

class UserRepository final {
public:
    bool ensureDefaultUsers(QString* errorMessage = nullptr);
    QVector<upkun::domain::User> enabledUsers() const;
    std::optional<upkun::domain::User> findEnabledByLoginName(const QString& loginName) const;
    std::optional<upkun::domain::User> authenticate(const QString& loginName, const QString& password, QString* errorMessage = nullptr);
    bool updateLastLogin(int userId, QString* errorMessage = nullptr);

private:
    static QString hashPassword(const QString& loginName, const QString& password);
};

} // namespace upkun::storage
