#include "storage/BatchRepository.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {

QString nowIso()
{
    return QDateTime::currentDateTime().toString(Qt::ISODate);
}

QString formatRate(int goodCount, int totalCount)
{
    if (totalCount <= 0) {
        return QStringLiteral("0.00%");
    }
    return QStringLiteral("%1%").arg(goodCount * 100.0 / totalCount, 0, 'f', 2);
}

upkun::domain::ProductionBatch batchFromQuery(const QSqlQuery& query)
{
    upkun::domain::ProductionBatch batch;
    batch.id = query.value(QStringLiteral("id")).toInt();
    batch.batchNo = query.value(QStringLiteral("batch_no")).toString();
    batch.recipeName = query.value(QStringLiteral("recipe_name")).toString();
    batch.operatorUserId = query.value(QStringLiteral("operator_user_id")).toInt();
    batch.operatorLoginName = query.value(QStringLiteral("operator_login_name")).toString();
    batch.operatorDisplayName = query.value(QStringLiteral("operator_display_name")).toString();
    batch.targetCount = query.value(QStringLiteral("target_count")).toInt();
    batch.startTotalCount = query.value(QStringLiteral("start_total_count")).toInt();
    batch.startGoodCount = query.value(QStringLiteral("start_good_count")).toInt();
    batch.startBadCount = query.value(QStringLiteral("start_bad_count")).toInt();
    batch.totalCount = query.value(QStringLiteral("total_count")).toInt();
    batch.goodCount = query.value(QStringLiteral("good_count")).toInt();
    batch.badCount = query.value(QStringLiteral("bad_count")).toInt();
    batch.status = upkun::domain::batchStatusFromCode(query.value(QStringLiteral("status")).toString());

    const QString startedAt = query.value(QStringLiteral("started_at")).toString();
    if (!startedAt.isEmpty()) {
        batch.startedAt = QDateTime::fromString(startedAt, Qt::ISODate);
    }
    const QString endedAt = query.value(QStringLiteral("ended_at")).toString();
    if (!endedAt.isEmpty()) {
        batch.endedAt = QDateTime::fromString(endedAt, Qt::ISODate);
    }
    return batch;
}

} // namespace

namespace upkun::storage {

bool BatchRepository::startBatch(upkun::domain::ProductionBatch* batch, QString* errorMessage)
{
    if (batch == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("批次对象为空");
        }
        return false;
    }

    if (activeBatch().has_value()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("已有进行中的批次");
        }
        return false;
    }

    batch->status = upkun::domain::BatchStatus::Running;
    batch->startedAt = QDateTime::currentDateTime();
    batch->endedAt = {};
    batch->totalCount = 0;
    batch->goodCount = 0;
    batch->badCount = 0;

    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO batches(batch_no, recipe_name, operator_user_id, operator_login_name, operator_display_name, "
        "target_count, start_total_count, start_good_count, start_bad_count, total_count, good_count, bad_count, "
        "status, started_at) "
        "VALUES(:batch_no, :recipe_name, :operator_user_id, :operator_login_name, :operator_display_name, "
        ":target_count, :start_total_count, :start_good_count, :start_bad_count, 0, 0, 0, :status, :started_at)"));
    query.bindValue(QStringLiteral(":batch_no"), batch->batchNo);
    query.bindValue(QStringLiteral(":recipe_name"), batch->recipeName);
    query.bindValue(QStringLiteral(":operator_user_id"), batch->operatorUserId);
    query.bindValue(QStringLiteral(":operator_login_name"), batch->operatorLoginName);
    query.bindValue(QStringLiteral(":operator_display_name"), batch->operatorDisplayName);
    query.bindValue(QStringLiteral(":target_count"), batch->targetCount);
    query.bindValue(QStringLiteral(":start_total_count"), batch->startTotalCount);
    query.bindValue(QStringLiteral(":start_good_count"), batch->startGoodCount);
    query.bindValue(QStringLiteral(":start_bad_count"), batch->startBadCount);
    query.bindValue(QStringLiteral(":status"), upkun::domain::batchStatusCode(batch->status));
    query.bindValue(QStringLiteral(":started_at"), batch->startedAt.toString(Qt::ISODate));

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    batch->id = query.lastInsertId().toInt();
    return true;
}

bool BatchRepository::finishActiveBatch(int batchId, int totalCount, int goodCount, int badCount, QString* errorMessage)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "UPDATE batches SET total_count = :total_count, good_count = :good_count, bad_count = :bad_count, "
        "status = :status, ended_at = :ended_at "
        "WHERE id = :id AND status = 'Running'"));
    query.bindValue(QStringLiteral(":total_count"), totalCount);
    query.bindValue(QStringLiteral(":good_count"), goodCount);
    query.bindValue(QStringLiteral(":bad_count"), badCount);
    query.bindValue(QStringLiteral(":status"), upkun::domain::batchStatusCode(upkun::domain::BatchStatus::Completed));
    query.bindValue(QStringLiteral(":ended_at"), nowIso());
    query.bindValue(QStringLiteral(":id"), batchId);

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    if (query.numRowsAffected() <= 0) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("未找到进行中的批次");
        }
        return false;
    }
    return true;
}

std::optional<upkun::domain::ProductionBatch> BatchRepository::activeBatch() const
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT * FROM batches WHERE status = 'Running' ORDER BY id DESC LIMIT 1"));
    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }
    return batchFromQuery(query);
}

QVector<QStringList> BatchRepository::recentRows(int limit) const
{
    QVector<QStringList> rows;
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT batch_no, recipe_name, operator_display_name, target_count, total_count, good_count, bad_count, "
        "status, started_at, COALESCE(ended_at, '') FROM batches ORDER BY id DESC LIMIT :limit"));
    query.bindValue(QStringLiteral(":limit"), limit);
    if (!query.exec()) {
        return rows;
    }

    while (query.next()) {
        const int totalCount = query.value(4).toInt();
        const int goodCount = query.value(5).toInt();
        QStringList row;
        row.append(query.value(0).toString());
        row.append(query.value(1).toString());
        row.append(query.value(2).toString());
        row.append(query.value(3).toString());
        row.append(QString::number(totalCount));
        row.append(QString::number(goodCount));
        row.append(query.value(6).toString());
        row.append(formatRate(goodCount, totalCount));
        row.append(upkun::domain::batchStatusText(upkun::domain::batchStatusFromCode(query.value(7).toString())));
        row.append(query.value(8).toString());
        row.append(query.value(9).toString());
        rows.append(row);
    }
    return rows;
}

} // namespace upkun::storage
