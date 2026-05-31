#include "storage/UserRepository.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {

upkun::domain::User userFromQuery(const QSqlQuery& query)
{
    // 仓储层负责把数据库行转换为领域模型，避免 UI 层直接依赖字段顺序。
    upkun::domain::User user;
    user.id = query.value(0).toInt();
    user.loginName = query.value(1).toString();
    user.displayName = query.value(2).toString();
    user.role = upkun::domain::userRoleFromCode(query.value(3).toString());
    user.enabled = query.value(4).toInt() != 0;

    const QString lastLoginAt = query.value(5).toString();
    if (!lastLoginAt.isEmpty()) {
        user.lastLoginAt = QDateTime::fromString(lastLoginAt, Qt::ISODate);
    }
    return user;
}

} // namespace

namespace upkun::storage {

bool UserRepository::ensureDefaultUsers(QString* errorMessage)
{
    // 默认用户是学习阶段的种子数据；方法保持幂等，重复启动不会重复插入。
    struct DefaultUser {
        QString loginName;
        QString displayName;
        upkun::domain::UserRole role;
        QString password;
    };

    const QVector<DefaultUser> users {
        { QStringLiteral("op001"), QStringLiteral("张三"), upkun::domain::UserRole::Operator, QStringLiteral("123456") },
        { QStringLiteral("eng001"), QStringLiteral("李工"), upkun::domain::UserRole::Engineer, QStringLiteral("123456") },
        { QStringLiteral("admin"), QStringLiteral("管理员"), upkun::domain::UserRole::Administrator, QStringLiteral("admin123") }
    };

    for (const auto& user : users) {
        QSqlQuery existsQuery;
        existsQuery.prepare(QStringLiteral("SELECT COUNT(1) FROM users WHERE login_name = :login_name"));
        existsQuery.bindValue(QStringLiteral(":login_name"), user.loginName);
        if (!existsQuery.exec() || !existsQuery.next()) {
            if (errorMessage != nullptr) {
                *errorMessage = existsQuery.lastError().text();
            }
            return false;
        }

        if (existsQuery.value(0).toInt() > 0) {
            continue;
        }

        QSqlQuery insertQuery;
        insertQuery.prepare(QStringLiteral(
            "INSERT INTO users(login_name, display_name, role, enabled, password_hash, created_at) "
            "VALUES(:login_name, :display_name, :role, 1, :password_hash, :created_at)"));
        insertQuery.bindValue(QStringLiteral(":login_name"), user.loginName);
        insertQuery.bindValue(QStringLiteral(":display_name"), user.displayName);
        insertQuery.bindValue(QStringLiteral(":role"), upkun::domain::userRoleCode(user.role));
        insertQuery.bindValue(QStringLiteral(":password_hash"), hashPassword(user.loginName, user.password));
        insertQuery.bindValue(QStringLiteral(":created_at"), QDateTime::currentDateTime().toString(Qt::ISODate));
        if (!insertQuery.exec()) {
            if (errorMessage != nullptr) {
                *errorMessage = insertQuery.lastError().text();
            }
            return false;
        }
    }

    return true;
}

QVector<upkun::domain::User> UserRepository::enabledUsers() const
{
    QVector<upkun::domain::User> users;
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT id, login_name, display_name, role, enabled, COALESCE(last_login_at, '') "
        "FROM users WHERE enabled = 1 ORDER BY id"));
    if (!query.exec()) {
        return users;
    }

    while (query.next()) {
        users.append(userFromQuery(query));
    }
    return users;
}

std::optional<upkun::domain::User> UserRepository::findEnabledByLoginName(const QString& loginName) const
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT id, login_name, display_name, role, enabled, COALESCE(last_login_at, '') "
        "FROM users WHERE login_name = :login_name AND enabled = 1"));
    query.bindValue(QStringLiteral(":login_name"), loginName.trimmed());
    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return userFromQuery(query);
}

std::optional<upkun::domain::User> UserRepository::authenticate(const QString& loginName, const QString& password, QString* errorMessage)
{
    const QString normalizedLoginName = loginName.trimmed();
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT id, login_name, display_name, role, enabled, COALESCE(last_login_at, ''), password_hash "
        "FROM users WHERE login_name = :login_name AND enabled = 1"));
    query.bindValue(QStringLiteral(":login_name"), normalizedLoginName);
    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return std::nullopt;
    }

    if (!query.next()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("用户不存在或已禁用");
        }
        return std::nullopt;
    }

    // 认证只返回领域用户，不把 password_hash 暴露给上层。
    const QString storedHash = query.value(6).toString();
    if (storedHash != hashPassword(normalizedLoginName, password)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("密码错误");
        }
        return std::nullopt;
    }

    auto user = userFromQuery(query);
    if (!updateLastLogin(user.id, errorMessage)) {
        return std::nullopt;
    }
    user.lastLoginAt = QDateTime::currentDateTime();
    return user;
}

bool UserRepository::updateLastLogin(int userId, QString* errorMessage)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE users SET last_login_at = :last_login_at WHERE id = :id"));
    query.bindValue(QStringLiteral(":last_login_at"), QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":id"), userId);
    if (query.exec()) {
        return true;
    }

    if (errorMessage != nullptr) {
        *errorMessage = query.lastError().text();
    }
    return false;
}

QString UserRepository::hashPassword(const QString& loginName, const QString& password)
{
    // 学习版用登录名参与哈希，避免明文保存；真实项目应升级为随机盐和密码策略。
    const QByteArray material = QStringLiteral("%1:%2").arg(loginName.trimmed(), password).toUtf8();
    return QString::fromLatin1(QCryptographicHash::hash(material, QCryptographicHash::Sha256).toHex());
}

} // namespace upkun::storage
