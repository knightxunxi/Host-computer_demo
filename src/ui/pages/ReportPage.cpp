#include "ui/pages/ReportPage.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace upkun::ui {

ReportPage::ReportPage(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(QStringLiteral(
        "QWidget { background: #ffffff; color: #000000; }"
        "QPushButton { background: #f5f5f5; color: #000000; border: 1px solid #a0a0a0; min-height: 32px; padding: 0 14px; }"
        "QPushButton:hover { background: #eeeeee; }"
        "QTableWidget { background: #ffffff; color: #000000; gridline-color: #d0d0d0; }"
        "QHeaderView::section { background: #f5f5f5; color: #000000; border: 1px solid #d0d0d0; padding: 4px; }"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    auto* title = new QLabel(QStringLiteral("报表中心"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    m_summaryLabel = new QLabel(QStringLiteral("汇总：暂无数据"), this);
    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 600; color: #000000;"));
    rootLayout->addWidget(m_summaryLabel);

    auto* buttons = new QHBoxLayout();
    auto* refreshButton = new QPushButton(QStringLiteral("刷新报表"), this);
    auto* exportButton = new QPushButton(QStringLiteral("导出 CSV"), this);
    buttons->addWidget(refreshButton);
    buttons->addWidget(exportButton);
    buttons->addStretch(1);
    rootLayout->addLayout(buttons);

    auto* tables = new QHBoxLayout();
    auto* left = new QVBoxLayout();
    auto* right = new QVBoxLayout();
    auto* batchTitle = new QLabel(QStringLiteral("批次报表"), this);
    auto* alarmTitle = new QLabel(QStringLiteral("报警统计"), this);
    auto* trendTitle = new QLabel(QStringLiteral("趋势日汇总"), this);
    batchTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700;"));
    alarmTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700;"));
    trendTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700;"));

    m_batchTable = createTable({
        QStringLiteral("批次"),
        QStringLiteral("配方"),
        QStringLiteral("操作员"),
        QStringLiteral("目标"),
        QStringLiteral("总数"),
        QStringLiteral("良品"),
        QStringLiteral("不良"),
        QStringLiteral("良率"),
        QStringLiteral("状态"),
        QStringLiteral("开始"),
        QStringLiteral("结束")
    });
    m_alarmTable = createTable({ QStringLiteral("工位"), QStringLiteral("等级"), QStringLiteral("次数") });
    m_trendTable = createTable({
        QStringLiteral("日期"),
        QStringLiteral("样本"),
        QStringLiteral("均速"),
        QStringLiteral("均灌装"),
        QStringLiteral("均重量")
    });

    left->addWidget(batchTitle);
    left->addWidget(m_batchTable, 1);
    right->addWidget(alarmTitle);
    right->addWidget(m_alarmTable, 1);
    right->addWidget(trendTitle);
    right->addWidget(m_trendTable, 1);
    tables->addLayout(left, 3);
    tables->addLayout(right, 2);
    rootLayout->addLayout(tables, 1);

    m_messageLabel = new QLabel(QStringLiteral("报表状态：未操作"), this);
    m_messageLabel->setStyleSheet(QStringLiteral("font-weight: 600; color: #000000;"));
    rootLayout->addWidget(m_messageLabel);

    connect(refreshButton, &QPushButton::clicked, this, &ReportPage::refreshRequested);
    connect(exportButton, &QPushButton::clicked, this, &ReportPage::exportRequested);
}

void ReportPage::setSummary(const upkun::storage::ReportSummary& summary)
{
    const double goodRate = summary.totalCount > 0 ? summary.goodCount * 100.0 / summary.totalCount : 0.0;
    m_summaryLabel->setText(QStringLiteral(
        "汇总：批次 %1 个，总产量 %2，良品 %3，不良 %4，良率 %5%，报警 %6 条，未闭环 %7 条，趋势样本 %8，平均速度 %9 pcs/min，平均重量 %10 g")
            .arg(summary.batchCount)
            .arg(summary.totalCount)
            .arg(summary.goodCount)
            .arg(summary.badCount)
            .arg(goodRate, 0, 'f', 2)
            .arg(summary.alarmCount)
            .arg(summary.openAlarmCount)
            .arg(summary.trendSampleCount)
            .arg(summary.averageSpeed, 0, 'f', 1)
            .arg(summary.averageWeight, 0, 'f', 1));
}

void ReportPage::setBatchRows(const QVector<QStringList>& rows)
{
    fillTable(m_batchTable, rows);
}

void ReportPage::setAlarmRows(const QVector<QStringList>& rows)
{
    fillTable(m_alarmTable, rows);
}

void ReportPage::setTrendRows(const QVector<QStringList>& rows)
{
    fillTable(m_trendTable, rows);
}

void ReportPage::setMessage(const QString& message)
{
    if (m_messageLabel != nullptr) {
        m_messageLabel->setText(QStringLiteral("报表状态：%1").arg(message));
    }
}

QTableWidget* ReportPage::createTable(const QStringList& headers)
{
    auto* table = new QTableWidget(this);
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    return table;
}

void ReportPage::fillTable(QTableWidget* table, const QVector<QStringList>& rows)
{
    if (table == nullptr) {
        return;
    }

    table->setRowCount(rows.size());
    for (qsizetype row = 0; row < rows.size(); ++row) {
        const auto& values = rows.at(row);
        for (qsizetype column = 0; column < values.size() && column < table->columnCount(); ++column) {
            auto* item = new QTableWidgetItem(values.at(column));
            item->setForeground(Qt::black);
            table->setItem(row, column, item);
        }
    }
}

} // namespace upkun::ui
