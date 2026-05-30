#include "ui/pages/MonitorPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace upkun::ui {

MonitorPage::MonitorPage(QWidget* parent)
    : QWidget(parent)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(18);

    auto* title = new QLabel(QStringLiteral("主监控"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #111827;"));
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
    grid->addWidget(createMetricCard(QStringLiteral("总产量"), QStringLiteral("0 pcs")), 0, 0);
    grid->addWidget(createMetricCard(QStringLiteral("良品数"), QStringLiteral("0 pcs")), 0, 1);
    grid->addWidget(createMetricCard(QStringLiteral("不良品数"), QStringLiteral("0 pcs")), 0, 2);
    grid->addWidget(createMetricCard(QStringLiteral("当前速度"), QStringLiteral("0 pcs/min")), 0, 3);
    grid->addWidget(createMetricCard(QStringLiteral("当前批次"), QStringLiteral("未开始")), 1, 0);
    grid->addWidget(createMetricCard(QStringLiteral("当前报警"), QStringLiteral("无")), 1, 1);
    grid->addWidget(createMetricCard(QStringLiteral("灌装量"), QStringLiteral("0 ml")), 1, 2);
    grid->addWidget(createMetricCard(QStringLiteral("当前重量"), QStringLiteral("0 g")), 1, 3);
    addControlButtons(grid);
    rootLayout->addLayout(grid);

    rootLayout->addStretch(1);
}

QWidget* MonitorPage::createStationCard(const QString& title, const QString& state)
{
    auto* card = new QFrame(this);
    card->setObjectName(QStringLiteral("stationCard"));
    card->setMinimumHeight(90);
    card->setStyleSheet(QStringLiteral(
        "#stationCard { background: white; border: 1px solid #d8dee6; border-radius: 6px; }"
        "QLabel { color: #111827; }"));

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(6);

    auto* nameLabel = new QLabel(title, card);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);

    auto* stateLabel = new QLabel(state, card);
    stateLabel->setAlignment(Qt::AlignCenter);
    stateLabel->setStyleSheet(QStringLiteral("color: #2563eb; font-weight: 600;"));

    layout->addWidget(nameLabel);
    layout->addStretch(1);
    layout->addWidget(stateLabel);

    return card;
}

QWidget* MonitorPage::createMetricCard(const QString& title, const QString& value)
{
    auto* card = new QFrame(this);
    card->setObjectName(QStringLiteral("metricCard"));
    card->setMinimumHeight(88);
    card->setStyleSheet(QStringLiteral(
        "#metricCard { background: #ffffff; border: 1px solid #d8dee6; border-radius: 6px; }"));

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 10, 14, 10);

    auto* titleLabel = new QLabel(title, card);
    titleLabel->setStyleSheet(QStringLiteral("color: #6b7280;"));
    auto* valueLabel = new QLabel(value, card);
    valueLabel->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 700; color: #111827;"));

    layout->addWidget(titleLabel);
    layout->addStretch(1);
    layout->addWidget(valueLabel);

    return card;
}

void MonitorPage::addControlButtons(QGridLayout* layout)
{
    auto* panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("controlPanel"));
    panel->setMinimumHeight(96);
    panel->setStyleSheet(QStringLiteral(
        "#controlPanel { background: #f8fafc; border: 1px solid #d8dee6; border-radius: 6px; }"
        "QPushButton { min-height: 34px; padding: 0 18px; font-weight: 600; }"));

    auto* row = new QHBoxLayout(panel);
    row->setContentsMargins(14, 12, 14, 12);
    row->setSpacing(10);

    row->addWidget(new QPushButton(QStringLiteral("启动"), panel));
    row->addWidget(new QPushButton(QStringLiteral("停止"), panel));
    row->addWidget(new QPushButton(QStringLiteral("复位"), panel));
    row->addWidget(new QPushButton(QStringLiteral("报警确认"), panel));
    row->addStretch(1);

    layout->addWidget(panel, 2, 0, 1, 4);
}

} // namespace upkun::ui
