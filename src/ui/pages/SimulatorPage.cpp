#include "ui/pages/SimulatorPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace upkun::ui {

namespace {

QString okText(bool ok, const QString& okText, const QString& badText)
{
    return ok ? okText : badText;
}

} // namespace

SimulatorPage::SimulatorPage(QWidget* parent)
    : QWidget(parent)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(18);
    setStyleSheet(QStringLiteral("QWidget { background: #ffffff; color: #000000; }"));

    auto* title = new QLabel(QStringLiteral("模拟器"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    auto* hint = new QLabel(QStringLiteral("用于启动虚拟 PLC、观察传感器/执行机构状态，并注入缺瓶、堵瓶、缺盖、重量异常、下料满料等模拟故障。"), this);
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("color: #000000;"));
    rootLayout->addWidget(hint);

    m_listeningLabel = new QLabel(QStringLiteral("监听状态：未启动"), this);
    m_alarmLabel = new QLabel(QStringLiteral("模拟报警：无"), this);
    m_countLabel = new QLabel(QStringLiteral("模拟产量：0"), this);
    m_processLabel = new QLabel(QStringLiteral("过程量：速度 0 pcs/min，灌装 0 ml，重量 0 g，扭矩 0 cN·m"), this);
    m_sensorLabel = new QLabel(QStringLiteral("传感器：上料OK | 灌装位无瓶 | 旋盖位无瓶 | 贴标位无瓶 | 下料通畅"), this);
    m_actuatorLabel = new QLabel(QStringLiteral("执行机构：输送停止 | 灌装阀OK | 理盖机OK | 标签机OK | 剔除气缸回位"), this);
    m_qualityLabel = new QLabel(QStringLiteral("质量模拟：最近产品合格，未触发剔除"), this);
    rootLayout->addWidget(m_listeningLabel);
    rootLayout->addWidget(m_alarmLabel);
    rootLayout->addWidget(m_countLabel);
    rootLayout->addWidget(m_processLabel);
    rootLayout->addWidget(m_sensorLabel);
    rootLayout->addWidget(m_actuatorLabel);
    rootLayout->addWidget(m_qualityLabel);

    auto* panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("simPanel"));
    panel->setStyleSheet(QStringLiteral(
        "#simPanel { background: #ffffff; border: 1px solid #d0d0d0; border-radius: 4px; }"
        "QPushButton { background: #f5f5f5; color: #000000; border: 1px solid #a0a0a0; min-height: 34px; padding: 0 16px; }"
        "QPushButton:hover { background: #eeeeee; }"
        "QPushButton:pressed { background: #dddddd; }"));

    auto* grid = new QGridLayout(panel);
    grid->setContentsMargins(16, 16, 16, 16);
    grid->setSpacing(12);
    auto* startButton = new QPushButton(QStringLiteral("启动模拟器"), panel);
    auto* stopButton = new QPushButton(QStringLiteral("停止模拟器"), panel);
    auto* estopFaultButton = new QPushButton(QStringLiteral("急停"), panel);
    auto* pressureFaultButton = new QPushButton(QStringLiteral("气压不足"), panel);
    auto* bottleFaultButton = new QPushButton(QStringLiteral("触发缺瓶"), panel);
    auto* jamFaultButton = new QPushButton(QStringLiteral("触发堵瓶"), panel);
    auto* capFaultButton = new QPushButton(QStringLiteral("触发缺盖"), panel);
    auto* weightFaultButton = new QPushButton(QStringLiteral("重量不合格"), panel);
    auto* labelFaultButton = new QPushButton(QStringLiteral("触发缺标签"), panel);
    auto* rejectFaultButton = new QPushButton(QStringLiteral("剔除失败"), panel);
    auto* outfeedFaultButton = new QPushButton(QStringLiteral("下料满料"), panel);
    auto* clearFaultButton = new QPushButton(QStringLiteral("清除模拟故障"), panel);

    connect(startButton, &QPushButton::clicked, this, &SimulatorPage::startSimulatorRequested);
    connect(stopButton, &QPushButton::clicked, this, &SimulatorPage::stopSimulatorRequested);
    connect(estopFaultButton, &QPushButton::clicked, this, [this] {
        emit faultRequested(1001);
    });
    connect(pressureFaultButton, &QPushButton::clicked, this, [this] {
        emit faultRequested(1003);
    });
    connect(bottleFaultButton, &QPushButton::clicked, this, [this] {
        emit faultRequested(2001);
    });
    connect(jamFaultButton, &QPushButton::clicked, this, [this] {
        emit faultRequested(2002);
    });
    connect(capFaultButton, &QPushButton::clicked, this, [this] {
        emit faultRequested(5001);
    });
    connect(weightFaultButton, &QPushButton::clicked, this, [this] {
        emit faultRequested(6002);
    });
    connect(labelFaultButton, &QPushButton::clicked, this, [this] {
        emit faultRequested(7001);
    });
    connect(rejectFaultButton, &QPushButton::clicked, this, [this] {
        emit faultRequested(8001);
    });
    connect(outfeedFaultButton, &QPushButton::clicked, this, [this] {
        emit faultRequested(9001);
    });
    connect(clearFaultButton, &QPushButton::clicked, this, &SimulatorPage::clearFaultRequested);

    grid->addWidget(startButton, 0, 0);
    grid->addWidget(stopButton, 0, 1);
    grid->addWidget(estopFaultButton, 1, 0);
    grid->addWidget(pressureFaultButton, 1, 1);
    grid->addWidget(bottleFaultButton, 2, 0);
    grid->addWidget(jamFaultButton, 2, 1);
    grid->addWidget(capFaultButton, 3, 0);
    grid->addWidget(weightFaultButton, 3, 1);
    grid->addWidget(labelFaultButton, 4, 0);
    grid->addWidget(rejectFaultButton, 4, 1);
    grid->addWidget(outfeedFaultButton, 5, 0);
    grid->addWidget(clearFaultButton, 5, 1);

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
    if (m_processLabel != nullptr) {
        m_processLabel->setText(QStringLiteral("过程量：速度 %1 pcs/min，灌装 %2 ml，重量 %3 g，扭矩 %4 cN·m")
                .arg(snapshot.counters.speed)
                .arg(snapshot.processValues.fillVolumeMl)
                .arg(snapshot.processValues.weightGram)
                .arg(snapshot.processValues.torqueCentinewtonMeter));
    }
    if (m_sensorLabel != nullptr) {
        m_sensorLabel->setText(QStringLiteral("传感器：%1 | %2 | %3 | %4 | %5")
                .arg(okText(snapshot.stationInputs.feedingMaterialReady, QStringLiteral("上料OK"), QStringLiteral("缺料/缺瓶")))
                .arg(okText(snapshot.stationInputs.bottleAtFilling, QStringLiteral("灌装位有瓶"), QStringLiteral("灌装位无瓶")))
                .arg(okText(snapshot.stationInputs.capPresent, QStringLiteral("旋盖有盖"), QStringLiteral("旋盖缺盖/无瓶")))
                .arg(okText(snapshot.stationInputs.bottleAtLabeling, QStringLiteral("贴标位有瓶"), QStringLiteral("贴标位无瓶")))
                .arg(okText(snapshot.stationInputs.outfeedReady && !snapshot.stationInputs.outfeedJam, QStringLiteral("下料通畅"), QStringLiteral("下料满料"))));
    }
    if (m_actuatorLabel != nullptr) {
        m_actuatorLabel->setText(QStringLiteral("执行机构：%1 | %2 | %3 | %4 | %5")
                .arg(okText(snapshot.stationInputs.conveyorRunning, QStringLiteral("输送运行"), QStringLiteral("输送停止")))
                .arg(okText(snapshot.stationInputs.fillingValveOk, QStringLiteral("灌装阀OK"), QStringLiteral("灌装阀异常")))
                .arg(okText(snapshot.stationInputs.capFeederReady, QStringLiteral("理盖机OK"), QStringLiteral("缺盖/理盖异常")))
                .arg(okText(snapshot.stationInputs.labelPrinterReady && snapshot.stationInputs.labelPaperOk, QStringLiteral("标签机OK"), QStringLiteral("标签/喷码异常")))
                .arg(okText(snapshot.stationInputs.rejectCylinderHome, QStringLiteral("剔除气缸回位"), QStringLiteral("剔除气缸异常"))));
    }
    if (m_qualityLabel != nullptr) {
        m_qualityLabel->setText(QStringLiteral("质量模拟：%1，%2")
                .arg(okText(snapshot.stationInputs.weightOk && !snapshot.stationInputs.weightNg, QStringLiteral("最近产品合格"), QStringLiteral("最近产品不合格")))
                .arg(okText(snapshot.stationInputs.rejectDetected, QStringLiteral("剔除信号已触发"), QStringLiteral("未触发剔除"))));
    }
}

void SimulatorPage::setListening(bool listening)
{
    if (m_listeningLabel != nullptr) {
        m_listeningLabel->setText(listening ? QStringLiteral("监听状态：%1").arg(m_endpoint) : QStringLiteral("监听状态：未启动"));
    }
}

void SimulatorPage::setEndpoint(const QString& endpoint)
{
    m_endpoint = endpoint.trimmed().isEmpty() ? QStringLiteral("127.0.0.1:1502") : endpoint.trimmed();
    if (m_listeningLabel != nullptr && m_listeningLabel->text() != QStringLiteral("监听状态：未启动")) {
        m_listeningLabel->setText(QStringLiteral("监听状态：%1").arg(m_endpoint));
    }
}

} // namespace upkun::ui
