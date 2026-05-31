#include "ui/pages/AlarmPage.h"

#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace upkun::ui {

AlarmPage::AlarmPage(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(QStringLiteral(
        "QWidget { background: #ffffff; color: #000000; }"
        "QTableWidget { background: #ffffff; color: #000000; gridline-color: #d0d0d0; }"
        "QHeaderView::section { background: #f5f5f5; color: #000000; border: 1px solid #d0d0d0; padding: 4px; }"
        "QPushButton { background: #f5f5f5; color: #000000; border: 1px solid #a0a0a0; min-height: 30px; padding: 0 14px; }"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    auto* title = new QLabel(QStringLiteral("报警记录"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    auto* refreshButton = new QPushButton(QStringLiteral("刷新记录"), this);
    connect(refreshButton, &QPushButton::clicked, this, &AlarmPage::refreshRequested);
    rootLayout->addWidget(refreshButton, 0, Qt::AlignLeft);

    auto* alarmTitle = new QLabel(QStringLiteral("最近报警"), this);
    alarmTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(alarmTitle);

    m_alarmTable = createTable({
        QStringLiteral("触发时间"),
        QStringLiteral("报警码"),
        QStringLiteral("报警名称"),
        QStringLiteral("工位"),
        QStringLiteral("等级"),
        QStringLiteral("状态"),
        QStringLiteral("确认人"),
        QStringLiteral("恢复时间")
    });
    rootLayout->addWidget(m_alarmTable, 1);

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
}

void AlarmPage::setAlarmRows(const QVector<QStringList>& rows)
{
    fillTable(m_alarmTable, rows);
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

} // namespace upkun::ui
