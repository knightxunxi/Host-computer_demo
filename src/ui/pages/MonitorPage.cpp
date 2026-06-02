#include "ui/pages/MonitorPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

QString stationStateText(const upkun::domain::DeviceSnapshot& snapshot, int stationIndex)
{
    if (snapshot.currentAlarmCode != 0 && static_cast<int>(snapshot.activeStation) == stationIndex) {
        return QStringLiteral("报警");
    }

    if (snapshot.systemState == upkun::domain::SystemState::Running
        && static_cast<int>(snapshot.activeStation) == stationIndex) {
        return QStringLiteral("运行中");
    }

    if (snapshot.systemState == upkun::domain::SystemState::Running) {
        return QStringLiteral("等待");
    }

    if (snapshot.systemState == upkun::domain::SystemState::Alarm) {
        return QStringLiteral("报警停机");
    }

    return QStringLiteral("待机");
}

QString stationStateMarkup(const upkun::domain::DeviceSnapshot& snapshot, int stationIndex)
{
    const QString text = stationStateText(snapshot, stationIndex);
    if (text == QStringLiteral("报警")) {
        return QStringLiteral("<span style=\"color:#d00000;\">报警</span>");
    }

    if (text == QStringLiteral("报警停机")) {
        return QStringLiteral("<span style=\"color:#d00000;\">报警</span><span style=\"color:#d18b00;\">停机</span>");
    }

    return text;
}

void setLabel(QLabel* label, const QString& value)
{
    if (label != nullptr) {
        label->setText(value);
    }
}

void setAlarmLabel(QLabel* label, int alarmCode)
{
    if (label == nullptr) {
        return;
    }

    label->setText(alarmCode == 0 ? QStringLiteral("无") : QString::number(alarmCode));
    label->setStyleSheet(alarmCode == 0
            ? QStringLiteral("background: transparent; font-size: 22px; font-weight: 700; color: #000000;")
            : QStringLiteral("background: transparent; font-size: 22px; font-weight: 700; color: #d00000;"));
}

} // namespace

namespace upkun::ui {

MonitorPage::MonitorPage(QWidget* parent)
    : QWidget(parent)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(18);
    setObjectName(QStringLiteral("monitorPage"));
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral("#monitorPage { background-color: #ffffff; color: #000000; }"));

    auto* title = new QLabel(QStringLiteral("主监控"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    auto* stationLayout = new QHBoxLayout();
    stationLayout->setSpacing(10);
    stationLayout->addWidget(createStationCard(QStringLiteral("上料"), QStringLiteral("待机")));
    stationLayout->addWidget(createStationCard(QStringLiteral("输送定位"), QStringLiteral("待机")));
    stationLayout->addWidget(createStationCard(QStringLiteral("灌装"), QStringLiteral("待机")));
    stationLayout->addWidget(createStationCard(QStringLiteral("旋盖"), QStringLiteral("待机")));
    stationLayout->addWidget(createStationCard(QStringLiteral("检测"), QStringLiteral("待机")));
    stationLayout->addWidget(createStationCard(QStringLiteral("贴标/喷码"), QStringLiteral("待机")));
    stationLayout->addWidget(createStationCard(QStringLiteral("分拣剔除"), QStringLiteral("待机")));
    stationLayout->addWidget(createStationCard(QStringLiteral("下料"), QStringLiteral("待机")));
    rootLayout->addLayout(stationLayout);

    auto* grid = new QGridLayout();
    grid->setSpacing(12);
    grid->addWidget(createMetricCard(QStringLiteral("总产量"), QStringLiteral("0 pcs"), &m_totalValueLabel), 0, 0);
    grid->addWidget(createMetricCard(QStringLiteral("良品数"), QStringLiteral("0 pcs"), &m_goodValueLabel), 0, 1);
    grid->addWidget(createMetricCard(QStringLiteral("不良品数"), QStringLiteral("0 pcs"), &m_badValueLabel), 0, 2);
    grid->addWidget(createMetricCard(QStringLiteral("当前速度"), QStringLiteral("0 pcs/min"), &m_speedValueLabel), 0, 3);
    grid->addWidget(createMetricCard(QStringLiteral("当前批次"), QStringLiteral("未开始"), &m_batchValueLabel), 1, 0);
    grid->addWidget(createMetricCard(QStringLiteral("当前报警"), QStringLiteral("无"), &m_alarmValueLabel), 1, 1);
    grid->addWidget(createMetricCard(QStringLiteral("灌装量"), QStringLiteral("0 ml"), &m_fillValueLabel), 1, 2);
    grid->addWidget(createMetricCard(QStringLiteral("当前重量"), QStringLiteral("0 g"), &m_weightValueLabel), 1, 3);
    addControlButtons(grid);
    rootLayout->addLayout(grid);

    rootLayout->addStretch(1);
}

void MonitorPage::updateSnapshot(const upkun::domain::DeviceSnapshot& snapshot)
{
    for (int i = 0; i < m_stationStateLabels.size(); ++i) {
        setLabel(m_stationStateLabels.at(i), stationStateMarkup(snapshot, i + 1));
    }

    setLabel(m_totalValueLabel, QStringLiteral("%1 pcs").arg(snapshot.counters.total));
    setLabel(m_goodValueLabel, QStringLiteral("%1 pcs").arg(snapshot.counters.good));
    setLabel(m_badValueLabel, QStringLiteral("%1 pcs").arg(snapshot.counters.bad));
    setLabel(m_speedValueLabel, QStringLiteral("%1 pcs/min").arg(snapshot.counters.speed));
    setLabel(m_batchValueLabel, snapshot.counters.batch > 0 ? QStringLiteral("%1 pcs").arg(snapshot.counters.batch) : QStringLiteral("未开始"));
    setAlarmLabel(m_alarmValueLabel, snapshot.currentAlarmCode);
    setLabel(m_fillValueLabel, QStringLiteral("%1 ml").arg(snapshot.processValues.fillVolumeMl));
    setLabel(m_weightValueLabel, QStringLiteral("%1 g").arg(snapshot.processValues.weightGram));
}

QWidget* MonitorPage::createStationCard(const QString& title, const QString& state)
{
    auto* card = new QFrame(this);
    card->setObjectName(QStringLiteral("stationCard"));
    card->setMinimumHeight(90);
    card->setStyleSheet(QStringLiteral(
        "#stationCard { background: #f2f2f2; border: 1px solid #d0d0d0; border-radius: 4px; }"
        "QLabel { background: transparent; color: #000000; }"));

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(6);

    auto* nameLabel = new QLabel(title, card);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);

    auto* stateLabel = new QLabel(state, card);
    stateLabel->setAlignment(Qt::AlignCenter);
    stateLabel->setTextFormat(Qt::RichText);
    stateLabel->setStyleSheet(QStringLiteral("background: transparent; color: #000000; font-weight: 600;"));
    m_stationStateLabels.append(stateLabel);

    layout->addWidget(nameLabel);
    layout->addStretch(1);
    layout->addWidget(stateLabel);

    return card;
}

QWidget* MonitorPage::createMetricCard(const QString& title, const QString& value, QLabel** valueLabel)
{
    auto* card = new QFrame(this);
    card->setObjectName(QStringLiteral("metricCard"));
    card->setMinimumHeight(88);
    card->setStyleSheet(QStringLiteral(
        "#metricCard { background: #f2f2f2; border: 1px solid #d0d0d0; border-radius: 4px; }"
        "QLabel { background: transparent; color: #000000; }"));

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 10, 14, 10);

    auto* titleLabel = new QLabel(title, card);
    titleLabel->setStyleSheet(QStringLiteral("background: transparent; color: #000000;"));
    auto* actualValueLabel = new QLabel(value, card);
    actualValueLabel->setStyleSheet(QStringLiteral("background: transparent; font-size: 22px; font-weight: 700; color: #000000;"));
    if (valueLabel != nullptr) {
        *valueLabel = actualValueLabel;
    }

    layout->addWidget(titleLabel);
    layout->addStretch(1);
    layout->addWidget(actualValueLabel);

    return card;
}

void MonitorPage::addControlButtons(QGridLayout* layout)
{
    auto* panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("controlPanel"));
    panel->setMinimumHeight(96);
    panel->setStyleSheet(QStringLiteral(
        "#controlPanel { background: #f2f2f2; border: 1px solid #d0d0d0; border-radius: 4px; }"
        "QPushButton { background: #ffffff; color: #000000; border: 1px solid #a0a0a0; min-height: 34px; padding: 0 18px; font-weight: 600; }"
        "QPushButton:hover { background: #eeeeee; }"
        "QPushButton:pressed { background: #dddddd; }"));

    auto* row = new QHBoxLayout(panel);
    row->setContentsMargins(14, 12, 14, 12);
    row->setSpacing(10);

    m_startButton = new QPushButton(QStringLiteral("启动"), panel);
    m_stopButton = new QPushButton(QStringLiteral("停止"), panel);
    m_resetButton = new QPushButton(QStringLiteral("复位"), panel);
    m_alarmAckButton = new QPushButton(QStringLiteral("报警确认"), panel);

    connect(m_startButton, &QPushButton::clicked, this, [this] {
        emit commandRequested(upkun::domain::DeviceCommand::Start);
    });
    connect(m_stopButton, &QPushButton::clicked, this, [this] {
        emit commandRequested(upkun::domain::DeviceCommand::Stop);
    });
    connect(m_resetButton, &QPushButton::clicked, this, [this] {
        emit commandRequested(upkun::domain::DeviceCommand::Reset);
    });
    connect(m_alarmAckButton, &QPushButton::clicked, this, [this] {
        emit commandRequested(upkun::domain::DeviceCommand::AlarmAck);
    });

    row->addWidget(m_startButton);
    row->addWidget(m_stopButton);
    row->addWidget(m_resetButton);
    row->addWidget(m_alarmAckButton);
    row->addStretch(1);

    layout->addWidget(panel, 2, 0, 1, 4);
}

} // namespace upkun::ui
