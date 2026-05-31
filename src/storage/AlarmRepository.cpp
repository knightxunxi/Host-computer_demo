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

QString stateText(const QString& state)
{
    if (state == QStringLiteral("ActiveUnacked")) {
        return QStringLiteral("进行中未确认");
    }
    if (state == QStringLiteral("ActiveAcked")) {
        return QStringLiteral("进行中已确认");
    }
    if (state == QStringLiteral("ClearedUnacked")) {
        return QStringLiteral("已恢复待确认");
    }
    if (state == QStringLiteral("Closed")) {
        return QStringLiteral("已关闭");
    }
    return state;
}

QString levelText(const QString& level)
{
    if (level == QStringLiteral("Critical")) {
        return QStringLiteral("严重");
    }
    if (level == QStringLiteral("Warning")) {
        return QStringLiteral("警告");
    }
    if (level == QStringLiteral("Error")) {
        return QStringLiteral("错误");
    }
    if (level == QStringLiteral("Info")) {
        return QStringLiteral("提示");
    }
    return level;
}

QString suggestionForCode(int code)
{
    switch (code) {
    case 1001:
        return QStringLiteral("检查急停按钮是否被按下，确认现场安全后复位急停并执行报警确认。");
    case 1002:
        return QStringLiteral("检查安全门、门磁和防护罩，确认关闭到位后复位报警。");
    case 1003:
        return QStringLiteral("检查气源压力、调压阀和气管漏气，压力恢复后再启动产线。");
    case 2001:
        return QStringLiteral("检查上料斗和进瓶轨道，补充瓶子并确认传感器信号恢复。");
    case 2002:
        return QStringLiteral("检查上料轨道是否堵瓶，清理卡滞物后低速试运行。");
    case 3001:
        return QStringLiteral("检查输送带、定位挡停和瓶身姿态，排除卡瓶后复位。");
    case 4001:
        return QStringLiteral("检查料液桶液位和供料泵，补料后确认灌装压力稳定。");
    case 4002:
        return QStringLiteral("检查灌装阀、流量计和灌装时间参数，必要时降低速度试运行。");
    case 5001:
        return QStringLiteral("检查理盖机和落盖轨道，补充瓶盖并确认缺盖传感器。");
    case 5002:
        return QStringLiteral("检查旋盖头扭矩、瓶盖姿态和夹瓶机构，调整后试运行。");
    case 6001:
        return QStringLiteral("检查检测相机/称重模块连接和供电，确认检测设备在线。");
    case 6002:
        return QStringLiteral("检查产品重量、灌装量和检测阈值，确认不合格品是否被正确剔除。");
    case 7001:
        return QStringLiteral("检查标签卷余量、标签传感器和放卷机构，补充标签后复位。");
    case 7002:
        return QStringLiteral("检查喷码机墨路、通讯状态和触发信号，确认喷码内容正常。");
    case 8001:
        return QStringLiteral("检查剔除气缸、气压和剔除确认传感器，确认不良品通道畅通。");
    case 9001:
        return QStringLiteral("检查下料盘或装箱工位是否满料，清空后恢复运行。");
    default:
        return QStringLiteral("查看对应工位传感器、执行机构和通信状态，确认现场安全后再复位。");
    }
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
    AlarmQueryFilter filter;
    filter.limit = limit;
    return queryRows(filter);
}

QVector<QStringList> AlarmRepository::queryRows(const AlarmQueryFilter& filter) const
{
    QVector<QStringList> rows;

    QStringList conditions;
    if (!filter.state.trimmed().isEmpty()) {
        conditions.append(QStringLiteral("state = :state"));
    }
    if (!filter.level.trimmed().isEmpty()) {
        conditions.append(QStringLiteral("level = :level"));
    }
    if (!filter.station.trimmed().isEmpty()) {
        conditions.append(QStringLiteral("station = :station"));
    }
    if (!filter.keyword.trimmed().isEmpty()) {
        conditions.append(QStringLiteral("(alarm_name LIKE :keyword OR station LIKE :keyword OR CAST(alarm_code AS TEXT) LIKE :keyword)"));
    }

    QString sql = QStringLiteral(
        "SELECT id, triggered_at, alarm_code, alarm_name, station, level, state, "
        "       COALESCE(acked_by, ''), COALESCE(acked_at, ''), COALESCE(cleared_at, ''), COALESCE(closed_at, '') "
        "FROM alarms");
    if (!conditions.isEmpty()) {
        sql.append(QStringLiteral(" WHERE "));
        sql.append(conditions.join(QStringLiteral(" AND ")));
    }
    sql.append(QStringLiteral(" ORDER BY id DESC LIMIT :limit"));

    QSqlQuery query;
    query.prepare(sql);
    if (!filter.state.trimmed().isEmpty()) {
        query.bindValue(QStringLiteral(":state"), filter.state.trimmed());
    }
    if (!filter.level.trimmed().isEmpty()) {
        query.bindValue(QStringLiteral(":level"), filter.level.trimmed());
    }
    if (!filter.station.trimmed().isEmpty()) {
        query.bindValue(QStringLiteral(":station"), filter.station.trimmed());
    }
    if (!filter.keyword.trimmed().isEmpty()) {
        query.bindValue(QStringLiteral(":keyword"), QStringLiteral("%%1%").arg(filter.keyword.trimmed()));
    }
    query.bindValue(QStringLiteral(":limit"), filter.limit > 0 ? filter.limit : 100);
    if (!query.exec()) {
        return rows;
    }

    while (query.next()) {
        QStringList row;
        row.append(query.value(0).toString());
        row.append(query.value(1).toString());
        row.append(query.value(2).toString());
        row.append(query.value(3).toString());
        row.append(query.value(4).toString());
        row.append(levelText(query.value(5).toString()));
        row.append(stateText(query.value(6).toString()));
        row.append(query.value(7).toString());
        row.append(query.value(8).toString());
        row.append(query.value(9).toString());
        row.append(query.value(10).toString());
        row.append(suggestionForCode(query.value(2).toInt()));
        rows.append(row);
    }
    return rows;
}

} // namespace upkun::storage
