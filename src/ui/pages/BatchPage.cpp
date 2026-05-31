#include "ui/pages/BatchPage.h"

#include <QDateTime>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

int activeGoodCount(const upkun::domain::ProductionBatch& batch, const upkun::domain::DeviceSnapshot& snapshot)
{
    return qMax(0, snapshot.counters.good - batch.startGoodCount);
}

int activeBadCount(const upkun::domain::ProductionBatch& batch, const upkun::domain::DeviceSnapshot& snapshot)
{
    return qMax(0, snapshot.counters.bad - batch.startBadCount);
}

} // namespace

namespace upkun::ui {

BatchPage::BatchPage(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(QStringLiteral(
        "QWidget { background: #ffffff; color: #000000; }"
        "QLineEdit, QSpinBox { background: #ffffff; color: #000000; border: 1px solid #a0a0a0; min-height: 28px; padding: 2px 6px; }"
        "QPushButton { background: #f5f5f5; color: #000000; border: 1px solid #a0a0a0; min-height: 32px; padding: 0 14px; }"
        "QPushButton:hover { background: #eeeeee; }"
        "QTableWidget { background: #ffffff; color: #000000; gridline-color: #d0d0d0; }"
        "QHeaderView::section { background: #f5f5f5; color: #000000; border: 1px solid #d0d0d0; padding: 4px; }"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    auto* title = new QLabel(QStringLiteral("批次管理"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    auto* formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setHorizontalSpacing(12);
    formLayout->setVerticalSpacing(8);

    m_batchNoEdit = new QLineEdit(generatedBatchNo(), this);
    m_targetCountSpin = new QSpinBox(this);
    m_targetCountSpin->setRange(1, 1000000);
    m_targetCountSpin->setValue(1000);
    m_targetCountSpin->setSuffix(QStringLiteral(" pcs"));
    m_contextRecipeLabel = new QLabel(QStringLiteral("当前配方：默认配方"), this);
    m_contextUserLabel = new QLabel(QStringLiteral("当前操作员：未登录"), this);

    formLayout->addRow(QStringLiteral("批次号"), m_batchNoEdit);
    formLayout->addRow(QStringLiteral("计划数量"), m_targetCountSpin);
    formLayout->addRow(QStringLiteral("配方"), m_contextRecipeLabel);
    formLayout->addRow(QStringLiteral("操作员"), m_contextUserLabel);
    rootLayout->addLayout(formLayout);

    auto* buttons = new QHBoxLayout();
    auto* generateButton = new QPushButton(QStringLiteral("生成批次号"), this);
    m_startButton = new QPushButton(QStringLiteral("开始批次"), this);
    m_endButton = new QPushButton(QStringLiteral("结束批次"), this);
    auto* refreshButton = new QPushButton(QStringLiteral("刷新记录"), this);
    buttons->addWidget(generateButton);
    buttons->addWidget(m_startButton);
    buttons->addWidget(m_endButton);
    buttons->addWidget(refreshButton);
    buttons->addStretch(1);
    rootLayout->addLayout(buttons);

    auto* grid = new QGridLayout();
    grid->setHorizontalSpacing(18);
    grid->setVerticalSpacing(8);
    m_activeBatchLabel = new QLabel(QStringLiteral("当前批次：无"), this);
    m_activeStatusLabel = new QLabel(QStringLiteral("状态：未开始"), this);
    m_activeRecipeLabel = new QLabel(QStringLiteral("配方：-"), this);
    m_activeTargetLabel = new QLabel(QStringLiteral("计划数量：0 pcs"), this);
    m_activeTotalLabel = new QLabel(QStringLiteral("批次数量：0 pcs"), this);
    m_activeGoodLabel = new QLabel(QStringLiteral("良品：0 pcs"), this);
    m_activeBadLabel = new QLabel(QStringLiteral("不良：0 pcs"), this);
    m_activeRateLabel = new QLabel(QStringLiteral("良率：0.00%"), this);
    grid->addWidget(m_activeBatchLabel, 0, 0);
    grid->addWidget(m_activeStatusLabel, 0, 1);
    grid->addWidget(m_activeRecipeLabel, 0, 2);
    grid->addWidget(m_activeTargetLabel, 0, 3);
    grid->addWidget(m_activeTotalLabel, 1, 0);
    grid->addWidget(m_activeGoodLabel, 1, 1);
    grid->addWidget(m_activeBadLabel, 1, 2);
    grid->addWidget(m_activeRateLabel, 1, 3);
    rootLayout->addLayout(grid);

    m_messageLabel = new QLabel(QStringLiteral("批次状态：未开始"), this);
    m_messageLabel->setStyleSheet(QStringLiteral("font-weight: 600; color: #000000;"));
    rootLayout->addWidget(m_messageLabel);

    auto* tableTitle = new QLabel(QStringLiteral("最近批次"), this);
    tableTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(tableTitle);

    m_batchTable = createTable({
        QStringLiteral("批次号"),
        QStringLiteral("配方"),
        QStringLiteral("操作员"),
        QStringLiteral("计划"),
        QStringLiteral("总数"),
        QStringLiteral("良品"),
        QStringLiteral("不良"),
        QStringLiteral("良率"),
        QStringLiteral("状态"),
        QStringLiteral("开始时间"),
        QStringLiteral("结束时间")
    });
    rootLayout->addWidget(m_batchTable, 1);

    connect(generateButton, &QPushButton::clicked, this, [this] {
        m_batchNoEdit->setText(generatedBatchNo());
    });
    connect(m_startButton, &QPushButton::clicked, this, [this] {
        const QString batchNo = m_batchNoEdit->text().trimmed().isEmpty() ? generatedBatchNo() : m_batchNoEdit->text().trimmed();
        m_batchNoEdit->setText(batchNo);
        emit startBatchRequested(batchNo, m_targetCountSpin->value());
    });
    connect(m_endButton, &QPushButton::clicked, this, &BatchPage::endBatchRequested);
    connect(refreshButton, &QPushButton::clicked, this, &BatchPage::refreshRequested);

    clearActiveBatch();
}

void BatchPage::setCurrentContext(const QString& recipeName, const QString& operatorName, int targetCount)
{
    m_contextRecipeLabel->setText(QStringLiteral("当前配方：%1").arg(recipeName));
    m_contextUserLabel->setText(QStringLiteral("当前操作员：%1").arg(operatorName));
    if (targetCount > 0) {
        m_targetCountSpin->setValue(targetCount);
    }
}

void BatchPage::setActiveBatch(const upkun::domain::ProductionBatch& batch, const upkun::domain::DeviceSnapshot& snapshot)
{
    const int totalCount = qMax(0, snapshot.counters.batch);
    const int goodCount = activeGoodCount(batch, snapshot);
    const int badCount = activeBadCount(batch, snapshot);

    m_activeBatchLabel->setText(QStringLiteral("当前批次：%1").arg(batch.batchNo));
    m_activeStatusLabel->setText(QStringLiteral("状态：%1").arg(upkun::domain::batchStatusText(batch.status)));
    m_activeRecipeLabel->setText(QStringLiteral("配方：%1").arg(batch.recipeName));
    m_activeTargetLabel->setText(QStringLiteral("计划数量：%1 pcs").arg(batch.targetCount));
    m_activeTotalLabel->setText(QStringLiteral("批次数量：%1 pcs").arg(totalCount));
    m_activeGoodLabel->setText(QStringLiteral("良品：%1 pcs").arg(goodCount));
    m_activeBadLabel->setText(QStringLiteral("不良：%1 pcs").arg(badCount));
    m_activeRateLabel->setText(QStringLiteral("良率：%1").arg(rateText(goodCount, totalCount)));
    m_startButton->setEnabled(false);
    m_endButton->setEnabled(true);
}

void BatchPage::clearActiveBatch()
{
    m_activeBatchLabel->setText(QStringLiteral("当前批次：无"));
    m_activeStatusLabel->setText(QStringLiteral("状态：未开始"));
    m_activeRecipeLabel->setText(QStringLiteral("配方：-"));
    m_activeTargetLabel->setText(QStringLiteral("计划数量：0 pcs"));
    m_activeTotalLabel->setText(QStringLiteral("批次数量：0 pcs"));
    m_activeGoodLabel->setText(QStringLiteral("良品：0 pcs"));
    m_activeBadLabel->setText(QStringLiteral("不良：0 pcs"));
    m_activeRateLabel->setText(QStringLiteral("良率：0.00%"));
    m_startButton->setEnabled(true);
    m_endButton->setEnabled(false);
}

void BatchPage::setBatchRows(const QVector<QStringList>& rows)
{
    fillTable(m_batchTable, rows);
}

void BatchPage::setMessage(const QString& message)
{
    m_messageLabel->setText(QStringLiteral("批次状态：%1").arg(message));
}

QTableWidget* BatchPage::createTable(const QStringList& headers)
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

void BatchPage::fillTable(QTableWidget* table, const QVector<QStringList>& rows)
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

QString BatchPage::generatedBatchNo() const
{
    return QStringLiteral("B%1").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMddHHmmss")));
}

QString BatchPage::rateText(int goodCount, int totalCount) const
{
    if (totalCount <= 0) {
        return QStringLiteral("0.00%");
    }
    return QStringLiteral("%1%").arg(goodCount * 100.0 / totalCount, 0, 'f', 2);
}

} // namespace upkun::ui
