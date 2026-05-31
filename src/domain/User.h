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

inline QString userRoleCode(UserRole role)
{
    switch (role) {
    case UserRole::Operator:
        return QStringLiteral("Operator");
    case UserRole::Engineer:
        return QStringLiteral("Engineer");
    case UserRole::Administrator:
        return QStringLiteral("Administrator");
    }
    return QStringLiteral("Operator");
}

inline QString userRoleText(UserRole role)
{
    switch (role) {
    case UserRole::Operator:
        return QStringLiteral("操作员");
    case UserRole::Engineer:
        return QStringLiteral("工程师");
    case UserRole::Administrator:
        return QStringLiteral("管理员");
    }
    return QStringLiteral("操作员");
}

inline UserRole userRoleFromCode(const QString& code)
{
    const QString normalized = code.trimmed();
    if (normalized == QStringLiteral("Engineer") || normalized == QStringLiteral("工程师")) {
        return UserRole::Engineer;
    }
    if (normalized == QStringLiteral("Administrator") || normalized == QStringLiteral("管理员")) {
        return UserRole::Administrator;
    }
    return UserRole::Operator;
}

} // namespace upkun::domain
