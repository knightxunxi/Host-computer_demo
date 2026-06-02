#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace upkun::storage {

struct ReportSummary {
    int batchCount = 0;
    int totalCount = 0;
    int goodCount = 0;
    int badCount = 0;
    int alarmCount = 0;
    int openAlarmCount = 0;
    int trendSampleCount = 0;
    double averageSpeed = 0.0;
    double averageWeight = 0.0;
};

class ReportRepository final {
public:
    ReportSummary summary() const;
    QVector<QStringList> batchRows(int limit = 100) const;
    QVector<QStringList> alarmSummaryRows() const;
    QVector<QStringList> trendSummaryRows() const;
    bool exportCsv(const QString& filePath, QString* errorMessage = nullptr) const;
};

} // namespace upkun::storage
