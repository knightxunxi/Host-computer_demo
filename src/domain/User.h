#pragma once

#include <QDateTime>
#include <QString>

namespace upkun::domain {

enum class UserRole {
    Operator,
    Engineer,
    Administrator
};

struct User {
    int id = 0;
    QString loginName;
    QString displayName;
    UserRole role = UserRole::Operator;
    bool enabled = true;
    QDateTime lastLoginAt;
};

} // namespace upkun::domain
