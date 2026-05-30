#include "app/MainWindow.h"

#include "ui/pages/MonitorPage.h"
#include "ui/pages/SimulatorPage.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

namespace upkun::app {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Upkun HMI - 小型包装产线上位机"));
    resize(1280, 800);

    auto* root = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    rootLayout->addWidget(createStatusHeader());

    auto* body = new QWidget(root);
    auto* bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    bodyLayout->addWidget(createNavigation());

    m_pages = new QStackedWidget(body);
    m_pages->addWidget(new upkun::ui::MonitorPage(m_pages));
    m_pages->addWidget(new upkun::ui::SimulatorPage(m_pages));
    bodyLayout->addWidget(m_pages, 1);

    rootLayout->addWidget(body, 1);
    rootLayout->addWidget(createAlarmFooter());

    setCentralWidget(root);

    connect(m_navigation, &QListWidget::currentRowChanged, this, &MainWindow::handleNavigationChanged);
    m_navigation->setCurrentRow(0);
}

void MainWindow::handleNavigationChanged(int row)
{
    if (row >= 0 && row < m_pages->count()) {
        m_pages->setCurrentIndex(row);
    }
}

QWidget* MainWindow::createStatusHeader()
{
    auto* header = new QFrame(this);
    header->setObjectName(QStringLiteral("statusHeader"));
    header->setMinimumHeight(64);
    header->setStyleSheet(QStringLiteral(
        "#statusHeader { background: #20252b; color: #f7f9fb; }"
        "QLabel { color: #f7f9fb; }"));

    auto* layout = new QHBoxLayout(header);
    layout->setContentsMargins(16, 10, 16, 10);
    layout->setSpacing(16);

    m_systemStateLabel = makeStatusLabel(QStringLiteral("系统状态"), QStringLiteral("待机"));
    m_modeLabel = makeStatusLabel(QStringLiteral("当前模式"), QStringLiteral("自动"));
    m_connectionLabel = makeStatusLabel(QStringLiteral("PLC通信"), QStringLiteral("未连接"));
    m_userLabel = makeStatusLabel(QStringLiteral("当前用户"), QStringLiteral("未登录"));

    layout->addWidget(m_systemStateLabel);
    layout->addWidget(m_modeLabel);
    layout->addWidget(m_connectionLabel);
    layout->addStretch(1);
    layout->addWidget(m_userLabel);

    return header;
}

QWidget* MainWindow::createNavigation()
{
    auto* frame = new QFrame(this);
    frame->setObjectName(QStringLiteral("navigationPanel"));
    frame->setFixedWidth(180);
    frame->setStyleSheet(QStringLiteral(
        "#navigationPanel { background: #f3f5f7; border-right: 1px solid #d6dce2; }"
        "QListWidget { background: transparent; border: none; font-size: 15px; }"
        "QListWidget::item { padding: 14px 16px; }"
        "QListWidget::item:selected { background: #dbeafe; color: #0f172a; }"));

    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(0, 12, 0, 12);

    m_navigation = new QListWidget(frame);
    m_navigation->addItem(new QListWidgetItem(QStringLiteral("主监控")));
    m_navigation->addItem(new QListWidgetItem(QStringLiteral("模拟器")));
    layout->addWidget(m_navigation);

    return frame;
}

QWidget* MainWindow::createAlarmFooter()
{
    auto* footer = new QFrame(this);
    footer->setObjectName(QStringLiteral("alarmFooter"));
    footer->setMinimumHeight(42);
    footer->setStyleSheet(QStringLiteral(
        "#alarmFooter { background: #fff7ed; border-top: 1px solid #fed7aa; }"
        "QLabel { color: #9a3412; font-weight: 600; }"));

    auto* layout = new QHBoxLayout(footer);
    layout->setContentsMargins(16, 0, 16, 0);

    m_alarmLabel = new QLabel(QStringLiteral("当前报警：无"), footer);
    layout->addWidget(m_alarmLabel);
    layout->addStretch(1);

    return footer;
}

QLabel* MainWindow::makeStatusLabel(const QString& title, const QString& value)
{
    auto* label = new QLabel(QStringLiteral("%1：%2").arg(title, value), this);
    label->setMinimumWidth(150);
    return label;
}

} // namespace upkun::app
