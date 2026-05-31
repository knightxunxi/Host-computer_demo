#include "storage/AlarmRepository.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {

QString nowIso()
{
    return QDateTime::currentDateTime().toString(Qt::ISODate);
}

bool execOrReport(QSqlQuery* query, QString* errorMessage)
{
    if (query->exec()) {
        return true;
    }
    if (errorMessage != nullptr) {
        *errorMessage = query->lastError().text();
    }
    return false;
}

bool hasOpenAlarm(int code)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT id FROM alarms "
        "WHERE alarm_code = :code AND state IN ('ActiveUnacked', 'ActiveAcked', 'ClearedUnacked') "
        "ORDER BY id DESC LIMIT 1"));
    query.bindValue(QStringLiteral(":code"), code);
    return query.exec() && query.next();
}

} // namespace

namespace upkun::storage {

bool AlarmRepository::openAlarm(int code, const QString& name, const QString& station, const QString& level, QString* errorMessage)
{
    if (hasOpenAlarm(code)) {
        return true;
    }

    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO alarms(alarm_code, alarm_name, station, level, state, triggered_at) "
        "VALUES(:code, :name, :station, :level, 'ActiveUnacked', :triggered_at)"));
    query.bindValue(QStringLiteral(":code"), code);
    query.bindValue(QStringLiteral(":name"), name);
    query.bindValue(QStringLiteral(":station"), station);
    query.bindValue(QStringLiteral(":level"), level);
    query.bindValue(QStringLiteral(":triggered_at"), nowIso());
    return execOrReport(&query, errorMessage);
}

bool AlarmRepository::acknowledgeOpenAlarm(int code, const QString& userName, QString* errorMessage)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "UPDATE alarms "
        "SET state = CASE WHEN state = 'ClearedUnacked' THEN 'Closed' ELSE 'ActiveAcked' END, "
        "    acked_at = COALESCE(acked_at, :acked_at), "
        "    acked_by = COALESCE(acked_by, :acked_by), "
        "    closed_at = CASE WHEN state = 'ClearedUnacked' THEN :closed_at ELSE closed_at END "
        "WHERE id = ("
        "    SELECT id FROM alarms "
        "    WHERE alarm_code = :code AND state IN ('ActiveUnacked', 'ActiveAcked', 'ClearedUnacked') "
        "    ORDER BY id DESC LIMIT 1"
        ")"));
    const QString timestamp = nowIso();
    query.bindValue(QStringLiteral(":acked_at"), timestamp);
    query.bindValue(QStringLiteral(":acked_by"), userName);
    query.bindValue(QStringLiteral(":closed_at"), timestamp);
    query.bindValue(QStringLiteral(":code"), code);
    return execOrReport(&query, errorMessage);
}

bool AlarmRepository::clearOpenAlarm(int code, QString* errorMessage)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "UPDATE alarms "
        "SET state = CASE WHEN state = 'ActiveAcked' THEN 'Closed' ELSE 'ClearedUnacked' END, "
        "    cleared_at = COALESCE(cleared_at, :cleared_at), "
        "    closed_at = CASE WHEN state = 'ActiveAcked' THEN :closed_at ELSE closed_at END "
        "WHERE id = ("
        "    SELECT id FROM alarms "
        "    WHERE alarm_code = :code AND state IN ('ActiveUnacked', 'ActiveAcked') "
        "    ORDER BY id DESC LIMIT 1"
        ")"));
    const QString timestamp = nowIso();
    query.bindValue(QStringLiteral(":cleared_at"), timestamp);
    query.bindValue(QStringLiteral(":closed_at"), timestamp);
    query.bindValue(QStringLiteral(":code"), code);
    return execOrReport(&query, errorMessage);
}

QVector<QStringList> AlarmRepository::recentRows(int limit) const
{
    QVector<QStringList> rows;
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT triggered_at, alarm_code, alarm_name, station, level, state, "
        "       COALESCE(acked_by, ''), COALESCE(cleared_at, '') "
        "FROM alarms ORDER BY id DESC LIMIT :limit"));
    query.bindValue(QStringLiteral(":limit"), limit);
    if (!query.exec()) {
        return rows;
    }

    while (query.next()) {
        QStringList row;
        for (int i = 0; i < 8; ++i) {
            row.append(query.value(i).toString());
        }
        rows.append(row);
    }
    return rows;
}

} // namespace upkun::storage
