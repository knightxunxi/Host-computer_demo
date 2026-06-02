#include "ui/pages/StationDetailPage.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {

QString okText(bool ok, const QString& okText, const QString& badText)
{
    return ok ? okText : badText;
}

QString activeText(const upkun::domain::DeviceSnapshot& snapshot, upkun::domain::Station station)
{
    if (snapshot.currentAlarmCode != 0 && snapshot.activeStation == station) {
        return QStringLiteral("报警停机");
    }
    if (snapshot.systemState == upkun::domain::SystemState::Running && snapshot.activeStation == station) {
        return QStringLiteral("运行中");
    }
    if (snapshot.systemState == upkun::domain::SystemState::Running) {
        return QStringLiteral("等待");
    }
    return QStringLiteral("待机");
}

} // namespace

namespace upkun::ui {

StationDetailPage::StationDetailPage(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(QStringLiteral(
        "QWidget { background: #ffffff; color: #000000; }"
        "QTableWidget { background: #ffffff; color: #000000; gridline-color: #d0d0d0; }"
        "QHeaderView::section { background: #f5f5f5; color: #000000; border: 1px solid #d0d0d0; padding: 4px; }"
        "QPushButton { background: #f5f5f5; color: #000000; border: 1px solid #a0a0a0; min-height: 32px; padding: 0 14px; }"
        "QPushButton:hover { background: #eeeeee; }"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    auto* title = new QLabel(QStringLiteral("工位详情"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    auto* hint = new QLabel(QStringLiteral("按工位查看传感器、执行机构和当前状态。手动调试命令需要工程师及以上权限。"), this);
    hint->setWordWrap(true);
    rootLayout->addWidget(hint);

    m_stationTable = new QTableWidget(8, 4, this);
    m_stationTable->setHorizontalHeaderLabels({
        QStringLiteral("工位"),
        QStringLiteral("传感器"),
        QStringLiteral("执行机构"),
        QStringLiteral("状态")
    });
    m_stationTable->verticalHeader()->setVisible(false);
    m_stationTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_stationTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_stationTable->horizontalHeader()->setStretchLastSection(true);
    m_stationTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    rootLayout->addWidget(m_stationTable, 1);

    auto* buttons = new QHBoxLayout();
    auto* autoButton = new QPushButton(QStringLiteral("切到自动"), this);
    auto* manualButton = new QPushButton(QStringLiteral("切到手动"), this);
    auto* rejectTestButton = new QPushButton(QStringLiteral("剔除测试"), this);
    buttons->addWidget(autoButton);
    buttons->addWidget(manualButton);
    buttons->addWidget(rejectTestButton);
    buttons->addStretch(1);
    rootLayout->addLayout(buttons);

    m_messageLabel = new QLabel(QStringLiteral("调试状态：未操作"), this);
    m_messageLabel->setStyleSheet(QStringLiteral("font-weight: 600; color: #000000;"));
    rootLayout->addWidget(m_messageLabel);

    connect(autoButton, &QPushButton::clicked, this, [this] {
        emit commandRequested(upkun::domain::DeviceCommand::ModeAuto);
    });
    connect(manualButton, &QPushButton::clicked, this, [this] {
        emit commandRequested(upkun::domain::DeviceCommand::ModeManual);
    });
    connect(rejectTestButton, &QPushButton::clicked, this, [this] {
        emit commandRequested(upkun::domain::DeviceCommand::RejectTest);
    });
}

void StationDetailPage::updateSnapshot(const upkun::domain::DeviceSnapshot& snapshot)
{
    setRow(0, QStringLiteral("上料"),
        okText(snapshot.stationInputs.feedingMaterialReady, QStringLiteral("料瓶就绪"), QStringLiteral("缺料/缺瓶")),
        QStringLiteral("上料机构"),
        activeText(snapshot, upkun::domain::Station::Feeding));
    setRow(1, QStringLiteral("输送定位"),
        okText(snapshot.stationInputs.conveyorRunning, QStringLiteral("输送信号运行"), QStringLiteral("输送停止")),
        QStringLiteral("输送带、挡停机构"),
        activeText(snapshot, upkun::domain::Station::Conveying));
    setRow(2, QStringLiteral("灌装"),
        okText(snapshot.stationInputs.bottleAtFilling, QStringLiteral("灌装位有瓶"), QStringLiteral("灌装位无瓶")),
        okText(snapshot.stationInputs.fillingValveOk, QStringLiteral("灌装阀正常"), QStringLiteral("灌装阀异常")),
        activeText(snapshot, upkun::domain::Station::Filling));
    setRow(3, QStringLiteral("旋盖"),
        okText(snapshot.stationInputs.capPresent, QStringLiteral("瓶盖到位"), QStringLiteral("缺盖/无瓶")),
        okText(snapshot.stationInputs.capFeederReady, QStringLiteral("理盖机正常"), QStringLiteral("理盖异常")),
        activeText(snapshot, upkun::domain::Station::Capping));
    setRow(4, QStringLiteral("检测"),
        okText(snapshot.stationInputs.scaleReady, QStringLiteral("检测模块就绪"), QStringLiteral("检测模块异常")),
        okText(snapshot.stationInputs.weightOk && !snapshot.stationInputs.weightNg, QStringLiteral("质量合格"), QStringLiteral("质量不合格")),
        activeText(snapshot, upkun::domain::Station::Inspecting));
    setRow(5, QStringLiteral("贴标/喷码"),
        okText(snapshot.stationInputs.bottleAtLabeling, QStringLiteral("贴标位有瓶"), QStringLiteral("贴标位无瓶")),
        okText(snapshot.stationInputs.labelPrinterReady && snapshot.stationInputs.labelPaperOk, QStringLiteral("标签/喷码正常"), QStringLiteral("标签/喷码异常")),
        activeText(snapshot, upkun::domain::Station::Labeling));
    setRow(6, QStringLiteral("分拣剔除"),
        okText(snapshot.stationInputs.rejectDetected, QStringLiteral("剔除信号触发"), QStringLiteral("未触发剔除")),
        okText(snapshot.stationInputs.rejectCylinderHome, QStringLiteral("剔除气缸回位"), QStringLiteral("剔除气缸异常")),
        activeText(snapshot, upkun::domain::Station::Rejecting));
    setRow(7, QStringLiteral("下料"),
        okText(snapshot.stationInputs.outfeedReady && !snapshot.stationInputs.outfeedJam, QStringLiteral("下料通畅"), QStringLiteral("下料满料/堵塞")),
        QStringLiteral("下料输送、装箱工位"),
        activeText(snapshot, upkun::domain::Station::Outfeeding));
}

void StationDetailPage::setMessage(const QString& message)
{
    if (m_messageLabel != nullptr) {
        m_messageLabel->setText(QStringLiteral("调试状态：%1").arg(message));
    }
}

void StationDetailPage::setRow(int row, const QString& station, const QString& sensor, const QString& actuator, const QString& status)
{
    const QStringList values { station, sensor, actuator, status };
    for (int column = 0; column < values.size(); ++column) {
        auto* item = m_stationTable->item(row, column);
        if (item == nullptr) {
            item = new QTableWidgetItem();
            item->setForeground(Qt::black);
            m_stationTable->setItem(row, column, item);
        }
        item->setText(values.at(column));
        item->setForeground(status.contains(QStringLiteral("报警")) && column == 3 ? Qt::red : Qt::black);
    }
}

} // namespace upkun::ui
