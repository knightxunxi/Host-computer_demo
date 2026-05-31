#include "ui/pages/SimulatorPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace upkun::ui {

SimulatorPage::SimulatorPage(QWidget* parent)
    : QWidget(parent)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(18);

    auto* title = new QLabel(QStringLiteral("模拟器"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #111827;"));
    rootLayout->addWidget(title);

    auto* hint = new QLabel(QStringLiteral("后续这里用于启动虚拟 PLC、设置合格率、触发缺瓶/堵瓶/缺盖/急停等模拟故障。"), this);
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("color: #4b5563;"));
    rootLayout->addWidget(hint);

    m_listeningLabel = new QLabel(QStringLiteral("监听状态：未启动"), this);
    m_alarmLabel = new QLabel(QStringLiteral("模拟报警：无"), this);
    m_countLabel = new QLabel(QStringLiteral("模拟产量：0"), this);
    rootLayout->addWidget(m_listeningLabel);
    rootLayout->addWidget(m_alarmLabel);
    rootLayout->addWidget(m_countLabel);

    auto* panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("simPanel"));
    panel->setStyleSheet(QStringLiteral(
        "#simPanel { background: #ffffff; border: 1px solid #d8dee6; border-radius: 6px; }"
        "QPushButton { min-height: 34px; padding: 0 16px; }"));

    auto* grid = new QGridLayout(panel);
    grid->setContentsMargins(16, 16, 16, 16);
    grid->setSpacing(12);
    auto* startButton = new QPushButton(QStringLiteral("启动模拟器"), panel);
    auto* stopButton = new QPushButton(QStringLiteral("停止模拟器"), panel);
    auto* bottleFaultButton = new QPushButton(QStringLiteral("触发缺瓶"), panel);
    auto* capFaultButton = new QPushButton(QStringLiteral("触发缺盖"), panel);
    auto* labelFaultButton = new QPushButton(QStringLiteral("触发缺标签"), panel);
    auto* clearFaultButton = new QPushButton(QStringLiteral("清除模拟故障"), panel);

    connect(startButton, &QPushButton::clicked, this, &SimulatorPage::startSimulatorRequested);
    connect(stopButton, &QPushButton::clicked, this, &SimulatorPage::stopSimulatorRequested);
    connect(bottleFaultButton, &QPushButton::clicked, this, [this] {
        emit faultRequested(2001);
    });
    connect(capFaultButton, &QPushButton::clicked, this, [this] {
        emit faultRequested(5001);
    });
    connect(labelFaultButton, &QPushButton::clicked, this, [this] {
        emit faultRequested(7001);
    });
    connect(clearFaultButton, &QPushButton::clicked, this, &SimulatorPage::clearFaultRequested);

    grid->addWidget(startButton, 0, 0);
    grid->addWidget(stopButton, 0, 1);
    grid->addWidget(bottleFaultButton, 1, 0);
    grid->addWidget(capFaultButton, 1, 1);
    grid->addWidget(labelFaultButton, 2, 0);
    grid->addWidget(clearFaultButton, 2, 1);

    rootLayout->addWidget(panel);
    rootLayout->addStretch(1);
}

void SimulatorPage::updateSnapshot(const upkun::domain::DeviceSnapshot& snapshot)
{
    if (m_alarmLabel != nullptr) {
        m_alarmLabel->setText(snapshot.currentAlarmCode == 0
                ? QStringLiteral("模拟报警：无")
                : QStringLiteral("模拟报警：%1").arg(snapshot.currentAlarmCode));
    }
    if (m_countLabel != nullptr) {
        m_countLabel->setText(QStringLiteral("模拟产量：%1").arg(snapshot.counters.total));
    }
}

void SimulatorPage::setListening(bool listening)
{
    if (m_listeningLabel != nullptr) {
        m_listeningLabel->setText(listening ? QStringLiteral("监听状态：127.0.0.1:1502") : QStringLiteral("监听状态：未启动"));
    }
}

} // namespace upkun::ui
