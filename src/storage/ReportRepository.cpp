#include "storage/ReportRepository.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlQuery>
#include <QTextStream>
#include <QVariant>

namespace {

QString csvEscape(QString value)
{
    value.replace(QStringLiteral("\""), QStringLiteral("\"\""));
    return QStringLiteral("\"%1\"").arg(value);
}

bool ensureParentDir(const QString& filePath, QString* errorMessage)
{
    const QFileInfo fileInfo(filePath);
    const QDir dir = fileInfo.absoluteDir();
    if (dir.exists() || QDir().mkpath(dir.absolutePath())) {
        return true;
    }
    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("无法创建目录：%1").arg(dir.absolutePath());
    }
    return false;
}

QString percentText(int goodCount, int totalCount)
{
    if (totalCount <= 0) {
        return QStringLiteral("0.00%");
    }
    return QStringLiteral("%1%").arg(goodCount * 100.0 / totalCount, 0, 'f', 2);
}

void writeSection(QTextStream* stream, const QString& title, const QStringList& headers, const QVector<QStringList>& rows)
{
    *stream << title << '\n';
    *stream << headers.join(QLatin1Char(',')) << '\n';
    for (const auto& row : rows) {
        QStringList values;
        for (const QString& value : row) {
            values.append(csvEscape(value));
        }
        *stream << values.join(QLatin1Char(',')) << '\n';
    }
    *stream << '\n';
}

} // namespace

namespace upkun::storage {

ReportSummary ReportRepository::summary() const
{
    ReportSummary result;

    QSqlQuery batchQuery(QStringLiteral(
        "SELECT COUNT(*), COALESCE(SUM(total_count), 0), COALESCE(SUM(good_count), 0), COALESCE(SUM(bad_count), 0) "
        "FROM batches"));
    if (batchQuery.next()) {
        result.batchCount = batchQuery.value(0).toInt();
        result.totalCount = batchQuery.value(1).toInt();
        result.goodCount = batchQuery.value(2).toInt();
        result.badCount = batchQuery.value(3).toInt();
    }

    QSqlQuery alarmQuery(QStringLiteral(
        "SELECT COUNT(*), "
        "SUM(CASE WHEN state IN ('ActiveUnacked', 'ActiveAcked', 'ClearedUnacked') THEN 1 ELSE 0 END) "
        "FROM alarms"));
    if (alarmQuery.next()) {
        result.alarmCount = alarmQuery.value(0).toInt();
        result.openAlarmCount = alarmQuery.value(1).toInt();
    }

    QSqlQuery trendQuery(QStringLiteral(
        "SELECT COUNT(*), COALESCE(AVG(speed), 0), COALESCE(AVG(weight), 0) "
        "FROM trend_samples"));
    if (trendQuery.next()) {
        result.trendSampleCount = trendQuery.value(0).toInt();
        result.averageSpeed = trendQuery.value(1).toDouble();
        result.averageWeight = trendQuery.value(2).toDouble();
    }

    return result;
}

QVector<QStringList> ReportRepository::batchRows(int limit) const
{
    QVector<QStringList> rows;
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT batch_no, recipe_name, operator_display_name, target_count, total_count, good_count, bad_count, status, started_at, COALESCE(ended_at, '') "
        "FROM batches ORDER BY id DESC LIMIT :limit"));
    query.bindValue(QStringLiteral(":limit"), limit > 0 ? limit : 100);
    if (!query.exec()) {
        return rows;
    }

    while (query.next()) {
        const int total = query.value(4).toInt();
        const int good = query.value(5).toInt();
        rows.append({
            query.value(0).toString(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString(),
            QString::number(total),
            QString::number(good),
            query.value(6).toString(),
            percentText(good, total),
            query.value(7).toString(),
            query.value(8).toString(),
            query.value(9).toString()
        });
    }
    return rows;
}

QVector<QStringList> ReportRepository::alarmSummaryRows() const
{
    QVector<QStringList> rows;
    QSqlQuery query(QStringLiteral(
        "SELECT station, level, COUNT(*) "
        "FROM alarms GROUP BY station, level ORDER BY COUNT(*) DESC, station ASC"));
    while (query.next()) {
        rows.append({ query.value(0).toString(), query.value(1).toString(), query.value(2).toString() });
    }
    return rows;
}

QVector<QStringList> ReportRepository::trendSummaryRows() const
{
    QVector<QStringList> rows;
    QSqlQuery query(QStringLiteral(
        "SELECT substr(sample_time, 1, 10), COUNT(*), COALESCE(AVG(speed), 0), COALESCE(AVG(fill_volume), 0), COALESCE(AVG(weight), 0) "
        "FROM trend_samples GROUP BY substr(sample_time, 1, 10) ORDER BY substr(sample_time, 1, 10) DESC LIMIT 30"));
    while (query.next()) {
        rows.append({
            query.value(0).toString(),
            query.value(1).toString(),
            QString::number(query.value(2).toDouble(), 'f', 1),
            QString::number(query.value(3).toDouble(), 'f', 1),
            QString::number(query.value(4).toDouble(), 'f', 1)
        });
    }
    return rows;
}

bool ReportRepository::exportCsv(const QString& filePath, QString* errorMessage) const
{
    if (!ensureParentDir(filePath, errorMessage)) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage != nullptr) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    const auto currentSummary = summary();
    stream << "summary,value\n";
    stream << "batch_count," << currentSummary.batchCount << '\n';
    stream << "total_count," << currentSummary.totalCount << '\n';
    stream << "good_count," << currentSummary.goodCount << '\n';
    stream << "bad_count," << currentSummary.badCount << '\n';
    stream << "alarm_count," << currentSummary.alarmCount << '\n';
    stream << "open_alarm_count," << currentSummary.openAlarmCount << '\n';
    stream << "trend_sample_count," << currentSummary.trendSampleCount << '\n';
    stream << "average_speed," << QString::number(currentSummary.averageSpeed, 'f', 2) << '\n';
    stream << "average_weight," << QString::number(currentSummary.averageWeight, 'f', 2) << "\n\n";

    writeSection(&stream, QStringLiteral("batch_rows"),
        { QStringLiteral("batch_no"), QStringLiteral("recipe"), QStringLiteral("operator"), QStringLiteral("target"),
            QStringLiteral("total"), QStringLiteral("good"), QStringLiteral("bad"), QStringLiteral("rate"),
            QStringLiteral("status"), QStringLiteral("started_at"), QStringLiteral("ended_at") },
        batchRows());
    writeSection(&stream, QStringLiteral("alarm_summary"),
        { QStringLiteral("station"), QStringLiteral("level"), QStringLiteral("count") },
        alarmSummaryRows());
    writeSection(&stream, QStringLiteral("trend_summary"),
        { QStringLiteral("date"), QStringLiteral("samples"), QStringLiteral("avg_speed"), QStringLiteral("avg_fill"), QStringLiteral("avg_weight") },
        trendSummaryRows());
    return true;
}

} // namespace upkun::storage
