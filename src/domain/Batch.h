#pragma once

#include <QDateTime>
#include <QString>

namespace upkun::domain {

enum class BatchStatus {
    Running,
    Completed,
    Aborted
};

struct ProductionBatch {
    int id = 0;
    QString batchNo;
    QString recipeName;
    int operatorUserId = 0;
    QString operatorLoginName;
    QString operatorDisplayName;
    int targetCount = 0;
    int startTotalCount = 0;
    int startGoodCount = 0;
    int startBadCount = 0;
    int totalCount = 0;
    int goodCount = 0;
    int badCount = 0;
    BatchStatus status = BatchStatus::Running;
    QDateTime startedAt;
    QDateTime endedAt;
};

inline QString batchStatusCode(BatchStatus status)
{
    switch (status) {
    case BatchStatus::Running:
        return QStringLiteral("Running");
    case BatchStatus::Completed:
        return QStringLiteral("Completed");
    case BatchStatus::Aborted:
        return QStringLiteral("Aborted");
    }
    return QStringLiteral("Running");
}

inline QString batchStatusText(BatchStatus status)
{
    switch (status) {
    case BatchStatus::Running:
        return QStringLiteral("进行中");
    case BatchStatus::Completed:
        return QStringLiteral("已完成");
    case BatchStatus::Aborted:
        return QStringLiteral("已中止");
    }
    return QStringLiteral("进行中");
}

inline BatchStatus batchStatusFromCode(const QString& code)
{
    const QString normalized = code.trimmed();
    if (normalized == QStringLiteral("Completed") || normalized == QStringLiteral("已完成")) {
        return BatchStatus::Completed;
    }
    if (normalized == QStringLiteral("Aborted") || normalized == QStringLiteral("已中止")) {
        return BatchStatus::Aborted;
    }
    return BatchStatus::Running;
}

} // namespace upkun::domain
