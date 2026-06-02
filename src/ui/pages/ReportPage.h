#pragma once

#include "storage/ReportRepository.h"

#include <QLabel>
#include <QTableWidget>
#include <QWidget>

namespace upkun::ui {

class ReportPage final : public QWidget {
    Q_OBJECT

public:
    explicit ReportPage(QWidget* parent = nullptr);

public slots:
    void setSummary(const upkun::storage::ReportSummary& summary);
    void setBatchRows(const QVector<QStringList>& rows);
    void setAlarmRows(const QVector<QStringList>& rows);
    void setTrendRows(const QVector<QStringList>& rows);
    void setMessage(const QString& message);

signals:
    void refreshRequested();
    void exportRequested();

private:
    QTableWidget* createTable(const QStringList& headers);
    void fillTable(QTableWidget* table, const QVector<QStringList>& rows);

    QLabel* m_summaryLabel = nullptr;
    QLabel* m_messageLabel = nullptr;
    QTableWidget* m_batchTable = nullptr;
    QTableWidget* m_alarmTable = nullptr;
    QTableWidget* m_trendTable = nullptr;
};

} // namespace upkun::ui
