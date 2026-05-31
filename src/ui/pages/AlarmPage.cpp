#include "ui/pages/AlarmPage.h"

#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {

constexpr int kColumnId = 0;
constexpr int kColumnTriggeredAt = 1;
constexpr int kColumnCode = 2;
constexpr int kColumnName = 3;
constexpr int kColumnStation = 4;
constexpr int kColumnLevel = 5;
constexpr int kColumnState = 6;
constexpr int kColumnAckedBy = 7;
constexpr int kColumnAckedAt = 8;
constexpr int kColumnClearedAt = 9;
constexpr int kColumnClosedAt = 10;
constexpr int kColumnSuggestion = 11;

QString itemText(QTableWidget* table, int row, int column)
{
    const auto* item = table != nullptr ? table->item(row, column) : nullptr;
    return item != nullptr ? item->text() : QString {};
}

} // namespace

namespace upkun::ui {

AlarmPage::AlarmPage(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(QStringLiteral(
        "QWidget { background: #ffffff; color: #000000; }"
        "QComboBox, QLineEdit { background: #ffffff; color: #000000; border: 1px solid #a0a0a0; min-height: 28px; padding: 2px 6px; }"
        "QTableWidget { background: #ffffff; color: #000000; gridline-color: #d0d0d0; }"
        "QHeaderView::section { background: #f5f5f5; color: #000000; border: 1px solid #d0d0d0; padding: 4px; }"
        "QPushButton { background: #f5f5f5; color: #000000; border: 1px solid #a0a0a0; min-height: 30px; padding: 0 14px; }"
        "QPushButton:hover { background: #eeeeee; }"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    auto* title = new QLabel(QStringLiteral("报警记录"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    auto* filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(8);
    m_stateFilter = new QComboBox(this);
    m_stateFilter->addItem(QStringLiteral("全部状态"), QString {});
    m_stateFilter->addItem(QStringLiteral("进行中未确认"), QStringLiteral("ActiveUnacked"));
    m_stateFilter->addItem(QStringLiteral("进行中已确认"), QStringLiteral("ActiveAcked"));
    m_stateFilter->addItem(QStringLiteral("已恢复待确认"), QStringLiteral("ClearedUnacked"));
    m_stateFilter->addItem(QStringLiteral("已关闭"), QStringLiteral("Closed"));

    m_levelFilter = new QComboBox(this);
    m_levelFilter->addItem(QStringLiteral("全部等级"), QString {});
    m_levelFilter->addItem(QStringLiteral("严重"), QStringLiteral("Critical"));
    m_levelFilter->addItem(QStringLiteral("警告"), QStringLiteral("Warning"));
    m_levelFilter->addItem(QStringLiteral("错误"), QStringLiteral("Error"));
    m_levelFilter->addItem(QStringLiteral("提示"), QStringLiteral("Info"));

    m_stationFilter = new QComboBox(this);
    m_stationFilter->addItem(QStringLiteral("全部工位"), QString {});
    for (const QString& station : {
             QStringLiteral("公共系统"),
             QStringLiteral("上料"),
             QStringLiteral("输送定位"),
             QStringLiteral("灌装"),
             QStringLiteral("旋盖"),
             QStringLiteral("检测"),
             QStringLiteral("贴标/喷码"),
             QStringLiteral("分拣剔除"),
             QStringLiteral("下料") }) {
        m_stationFilter->addItem(station, station);
    }

    m_keywordEdit = new QLineEdit(this);
    m_keywordEdit->setPlaceholderText(QStringLiteral("报警码/名称/工位"));
    auto* refreshButton = new QPushButton(QStringLiteral("刷新记录"), this);
    connect(refreshButton, &QPushButton::clicked, this, &AlarmPage::refreshRequested);
    filterLayout->addWidget(m_stateFilter);
    filterLayout->addWidget(m_levelFilter);
    filterLayout->addWidget(m_stationFilter);
    filterLayout->addWidget(m_keywordEdit, 1);
    filterLayout->addWidget(refreshButton);
    rootLayout->addLayout(filterLayout);

    auto* alarmTitle = new QLabel(QStringLiteral("最近报警"), this);
    alarmTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(alarmTitle);

    m_alarmTable = createTable({
        QStringLiteral("ID"),
        QStringLiteral("触发时间"),
        QStringLiteral("报警码"),
        QStringLiteral("报警名称"),
        QStringLiteral("工位"),
        QStringLiteral("等级"),
        QStringLiteral("状态"),
        QStringLiteral("确认人"),
        QStringLiteral("确认时间"),
        QStringLiteral("恢复时间"),
        QStringLiteral("关闭时间"),
        QStringLiteral("处理建议")
    });
    m_alarmTable->setColumnHidden(kColumnId, true);
    m_alarmTable->setColumnHidden(kColumnSuggestion, true);
    connect(m_alarmTable, &QTableWidget::currentCellChanged, this, [this](int row, int, int, int) {
        updateAlarmDetail(row);
    });
    rootLayout->addWidget(m_alarmTable, 1);

    auto* detailTitle = new QLabel(QStringLiteral("报警详情"), this);
    detailTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(detailTitle);

    auto* detailGrid = new QGridLayout();
    detailGrid->setHorizontalSpacing(18);
    detailGrid->setVerticalSpacing(6);
    m_detailTitleLabel = new QLabel(QStringLiteral("报警：未选择"), this);
    m_detailStateLabel = new QLabel(QStringLiteral("状态：-"), this);
    m_detailAckLabel = new QLabel(QStringLiteral("确认：-"), this);
    m_detailTimeLabel = new QLabel(QStringLiteral("时间：-"), this);
    m_detailSuggestionLabel = new QLabel(QStringLiteral("处理建议：-"), this);
    m_detailSuggestionLabel->setWordWrap(true);
    detailGrid->addWidget(m_detailTitleLabel, 0, 0);
    detailGrid->addWidget(m_detailStateLabel, 0, 1);
    detailGrid->addWidget(m_detailAckLabel, 0, 2);
    detailGrid->addWidget(m_detailTimeLabel, 1, 0, 1, 3);
    detailGrid->addWidget(m_detailSuggestionLabel, 2, 0, 1, 3);
    rootLayout->addLayout(detailGrid);

    auto* operationTitle = new QLabel(QStringLiteral("最近操作"), this);
    operationTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(operationTitle);

    m_operationTable = createTable({
        QStringLiteral("时间"),
        QStringLiteral("用户"),
        QStringLiteral("角色"),
        QStringLiteral("动作"),
        QStringLiteral("目标"),
        QStringLiteral("结果"),
        QStringLiteral("消息")
    });
    rootLayout->addWidget(m_operationTable, 1);

    connect(m_stateFilter, &QComboBox::currentIndexChanged, this, &AlarmPage::refreshRequested);
    connect(m_levelFilter, &QComboBox::currentIndexChanged, this, &AlarmPage::refreshRequested);
    connect(m_stationFilter, &QComboBox::currentIndexChanged, this, &AlarmPage::refreshRequested);
    connect(m_keywordEdit, &QLineEdit::returnPressed, this, &AlarmPage::refreshRequested);
}

QString AlarmPage::stateFilter() const
{
    return m_stateFilter != nullptr ? m_stateFilter->currentData().toString() : QString {};
}

QString AlarmPage::levelFilter() const
{
    return m_levelFilter != nullptr ? m_levelFilter->currentData().toString() : QString {};
}

QString AlarmPage::stationFilter() const
{
    return m_stationFilter != nullptr ? m_stationFilter->currentData().toString() : QString {};
}

QString AlarmPage::keywordFilter() const
{
    return m_keywordEdit != nullptr ? m_keywordEdit->text().trimmed() : QString {};
}

void AlarmPage::setAlarmRows(const QVector<QStringList>& rows)
{
    fillTable(m_alarmTable, rows);
    if (m_alarmTable != nullptr && m_alarmTable->rowCount() > 0) {
        m_alarmTable->setCurrentCell(0, kColumnTriggeredAt);
        updateAlarmDetail(0);
    } else {
        clearAlarmDetail();
    }
}

void AlarmPage::setOperationRows(const QVector<QStringList>& rows)
{
    fillTable(m_operationTable, rows);
}

QTableWidget* AlarmPage::createTable(const QStringList& headers)
{
    auto* table = new QTableWidget(this);
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    return table;
}

void AlarmPage::fillTable(QTableWidget* table, const QVector<QStringList>& rows)
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

void AlarmPage::updateAlarmDetail(int row)
{
    if (m_alarmTable == nullptr || row < 0 || row >= m_alarmTable->rowCount()) {
        clearAlarmDetail();
        return;
    }

    const QString code = itemText(m_alarmTable, row, kColumnCode);
    const QString name = itemText(m_alarmTable, row, kColumnName);
    const QString station = itemText(m_alarmTable, row, kColumnStation);
    const QString level = itemText(m_alarmTable, row, kColumnLevel);
    const QString state = itemText(m_alarmTable, row, kColumnState);
    const QString ackedBy = itemText(m_alarmTable, row, kColumnAckedBy);
    const QString ackedAt = itemText(m_alarmTable, row, kColumnAckedAt);
    const QString triggeredAt = itemText(m_alarmTable, row, kColumnTriggeredAt);
    const QString clearedAt = itemText(m_alarmTable, row, kColumnClearedAt);
    const QString closedAt = itemText(m_alarmTable, row, kColumnClosedAt);
    const QString suggestion = itemText(m_alarmTable, row, kColumnSuggestion);

    m_detailTitleLabel->setText(QStringLiteral("报警：%1 %2（%3，%4）").arg(code, name, station, level));
    m_detailStateLabel->setText(QStringLiteral("状态：%1").arg(state));
    m_detailAckLabel->setText(QStringLiteral("确认：%1 / %2")
            .arg(ackedBy.isEmpty() ? QStringLiteral("未确认") : ackedBy,
                ackedAt.isEmpty() ? QStringLiteral("无确认时间") : ackedAt));
    m_detailTimeLabel->setText(QStringLiteral("时间：触发 %1，恢复 %2，关闭 %3")
            .arg(triggeredAt,
                clearedAt.isEmpty() ? QStringLiteral("未恢复") : clearedAt,
                closedAt.isEmpty() ? QStringLiteral("未关闭") : closedAt));
    m_detailSuggestionLabel->setText(QStringLiteral("处理建议：%1").arg(suggestion));
}

void AlarmPage::clearAlarmDetail()
{
    m_detailTitleLabel->setText(QStringLiteral("报警：未选择"));
    m_detailStateLabel->setText(QStringLiteral("状态：-"));
    m_detailAckLabel->setText(QStringLiteral("确认：-"));
    m_detailTimeLabel->setText(QStringLiteral("时间：-"));
    m_detailSuggestionLabel->setText(QStringLiteral("处理建议：-"));
}

} // namespace upkun::ui
