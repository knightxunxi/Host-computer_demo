#include "storage/OperationLogRepository.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {

bool appendRow(const QVariant& userId,
    const QString& loginName,
    const QString& displayName,
    const QString& role,
    const QString& action,
    const QString& target,
    const QString& result,
    const QString& message,
    QString* errorMessage)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO operation_logs(user_id, login_name, display_name, role, action, target, result, message, created_at) "
        "VALUES(:user_id, :login_name, :display_name, :role, :action, :target, :result, :message, :created_at)"));
    query.bindValue(QStringLiteral(":user_id"), userId);
    query.bindValue(QStringLiteral(":login_name"), loginName);
    query.bindValue(QStringLiteral(":display_name"), displayName);
    query.bindValue(QStringLiteral(":role"), role);
    query.bindValue(QStringLiteral(":action"), action);
    query.bindValue(QStringLiteral(":target"), target);
    query.bindValue(QStringLiteral(":result"), result);
    query.bindValue(QStringLiteral(":message"), message);
    query.bindValue(QStringLiteral(":created_at"), QDateTime::currentDateTime().toString(Qt::ISODate));

    if (query.exec()) {
        return true;
    }
    if (errorMessage != nullptr) {
        *errorMessage = query.lastError().text();
    }
    return false;
}

} // namespace

namespace upkun::storage {

bool OperationLogRepository::append(const QString& action, const QString& target, const QString& result, const QString& message, QString* errorMessage)
{
    return appendRow(0,
        QStringLiteral("system"),
        QStringLiteral("系统"),
        QStringLiteral("System"),
        action,
        target,
        result,
        message,
        errorMessage);
}

bool OperationLogRepository::append(const upkun::domain::User& user, const QString& action, const QString& target, const QString& result, const QString& message, QString* errorMessage)
{
    return appendRow(user.id,
        user.loginName.isEmpty() ? QStringLiteral("unknown") : user.loginName,
        user.displayName.isEmpty() ? QStringLiteral("未命名用户") : user.displayName,
        upkun::domain::userRoleText(user.role),
        action,
        target,
        result,
        message,
        errorMessage);
}

QVector<QStringList> OperationLogRepository::recentRows(int limit) const
{
    QVector<QStringList> rows;
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT created_at, display_name, role, action, target, result, COALESCE(message, '') "
        "FROM operation_logs ORDER BY id DESC LIMIT :limit"));
    query.bindValue(QStringLiteral(":limit"), limit);
    if (!query.exec()) {
        return rows;
    }

    while (query.next()) {
        QStringList row;
        for (int i = 0; i < 7; ++i) {
            row.append(query.value(i).toString());
        }
        rows.append(row);
    }
    return rows;
}

} // namespace upkun::storage
