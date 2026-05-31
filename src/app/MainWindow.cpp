#include "app/MainWindow.h"

#include "device/ModbusTcpClient.h"
#include "simulator/SimulatedModbusServer.h"
#include "ui/pages/AlarmPage.h"
#include "ui/pages/MonitorPage.h"
#include "ui/pages/SimulatorPage.h"

#include <QFrame>
#include <QHostAddress>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

namespace {

QString systemStateText(upkun::domain::SystemState state)
{
    switch (state) {
    case upkun::domain::SystemState::Stopped:
        return QStringLiteral("停止");
    case upkun::domain::SystemState::Standby:
        return QStringLiteral("待机");
    case upkun::domain::SystemState::Running:
        return QStringLiteral("运行中");
    case upkun::domain::SystemState::Paused:
        return QStringLiteral("暂停");
    case upkun::domain::SystemState::Alarm:
        return QStringLiteral("报警");
    case upkun::domain::SystemState::EmergencyStop:
        return QStringLiteral("急停");
    case upkun::domain::SystemState::Resetting:
        return QStringLiteral("复位中");
    }
    return QStringLiteral("未知");
}

QString runModeText(upkun::domain::RunMode mode)
{
    switch (mode) {
    case upkun::domain::RunMode::Manual:
        return QStringLiteral("手动");
    case upkun::domain::RunMode::Auto:
        return QStringLiteral("自动");
    case upkun::domain::RunMode::Maintenance:
        return QStringLiteral("维护");
    case upkun::domain::RunMode::Unknown:
        return QStringLiteral("未知");
    }
    return QStringLiteral("未知");
}

QString connectionStateText(upkun::domain::ConnectionState state)
{
    switch (state) {
    case upkun::domain::ConnectionState::Disconnected:
        return QStringLiteral("断开");
    case upkun::domain::ConnectionState::Connecting:
        return QStringLiteral("连接中");
    case upkun::domain::ConnectionState::Connected:
        return QStringLiteral("已连接");
    case upkun::domain::ConnectionState::Reconnecting:
        return QStringLiteral("重连中");
    case upkun::domain::ConnectionState::Error:
        return QStringLiteral("错误");
    }
    return QStringLiteral("未知");
}

QString commandText(upkun::domain::DeviceCommand command)
{
    switch (command) {
    case upkun::domain::DeviceCommand::Start:
        return QStringLiteral("启动");
    case upkun::domain::DeviceCommand::Stop:
        return QStringLiteral("停止");
    case upkun::domain::DeviceCommand::Reset:
        return QStringLiteral("复位");
    case upkun::domain::DeviceCommand::AlarmAck:
        return QStringLiteral("报警确认");
    case upkun::domain::DeviceCommand::ModeAuto:
        return QStringLiteral("自动模式");
    case upkun::domain::DeviceCommand::ModeManual:
        return QStringLiteral("手动模式");
    case upkun::domain::DeviceCommand::BatchStart:
        return QStringLiteral("开始批次");
    case upkun::domain::DeviceCommand::BatchEnd:
        return QStringLiteral("结束批次");
    case upkun::domain::DeviceCommand::RejectTest:
        return QStringLiteral("剔除测试");
    case upkun::domain::DeviceCommand::SimFault:
        return QStringLiteral("模拟故障");
    }
    return QStringLiteral("命令");
}

} // namespace

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
    m_monitorPage = new upkun::ui::MonitorPage(m_pages);
    m_alarmPage = new upkun::ui::AlarmPage(m_pages);
    m_simulatorPage = new upkun::ui::SimulatorPage(m_pages);
    m_pages->addWidget(m_monitorPage);
    m_pages->addWidget(m_alarmPage);
    m_pages->addWidget(m_simulatorPage);
    bodyLayout->addWidget(m_pages, 1);

    rootLayout->addWidget(body, 1);
    rootLayout->addWidget(createAlarmFooter());

    setCentralWidget(root);

    connect(m_navigation, &QListWidget::currentRowChanged, this, &MainWindow::handleNavigationChanged);
    m_navigation->setCurrentRow(0);

    setupStorage();
    setupDeviceLink();
}

void MainWindow::handleNavigationChanged(int row)
{
    if (row >= 0 && row < m_pages->count()) {
        m_pages->setCurrentIndex(row);
    }
}

void MainWindow::handleSnapshotUpdated(const upkun::domain::DeviceSnapshot& snapshot)
{
    m_systemStateLabel->setText(QStringLiteral("系统状态：%1").arg(systemStateText(snapshot.systemState)));
    m_modeLabel->setText(QStringLiteral("当前模式：%1").arg(runModeText(snapshot.currentMode)));
    m_alarmLabel->setText(snapshot.currentAlarmCode == 0
            ? QStringLiteral("当前报警：无")
            : QStringLiteral("当前报警：%1").arg(snapshot.currentAlarmCode));

    if (m_alarmService != nullptr) {
        m_alarmService->processSnapshot(snapshot);
    }

    m_monitorPage->updateSnapshot(snapshot);
    m_simulatorPage->updateSnapshot(snapshot);
}

void MainWindow::handleConnectionChanged(upkun::domain::ConnectionState state)
{
    m_connectionLabel->setText(QStringLiteral("PLC通信：%1").arg(connectionStateText(state)));
}

void MainWindow::handleCommandFinished(upkun::domain::DeviceCommand command, bool ok, const QString& message)
{
    m_alarmLabel->setText(QStringLiteral("命令反馈：%1 %2，%3")
            .arg(commandText(command), ok ? QStringLiteral("成功") : QStringLiteral("失败"), message));
    appendOperationLog(commandText(command), QStringLiteral("PLC/模拟器"), ok ? QStringLiteral("成功") : QStringLiteral("失败"), message);

    if (ok && command == upkun::domain::DeviceCommand::AlarmAck && m_alarmService != nullptr) {
        m_alarmService->acknowledgeCurrentAlarm(QStringLiteral("系统"));
    }
    refreshAlarmRecords();
}

void MainWindow::refreshAlarmRecords()
{
    if (m_alarmPage == nullptr || m_alarmService == nullptr) {
        return;
    }

    m_alarmPage->setAlarmRows(m_alarmService->recentAlarmRows());
    m_alarmPage->setOperationRows(m_alarmService->recentOperationRows());
}

void MainWindow::startSimulator()
{
    QString errorMessage;
    if (!m_simulatedServer->start(QHostAddress::LocalHost, 1502, &errorMessage)) {
        m_alarmLabel->setText(QStringLiteral("模拟器启动失败：%1").arg(errorMessage));
        appendOperationLog(QStringLiteral("启动模拟器"), QStringLiteral("127.0.0.1:1502"), QStringLiteral("失败"), errorMessage);
        refreshAlarmRecords();
        return;
    }

    m_simulatorPage->setListening(true);
    appendOperationLog(QStringLiteral("启动模拟器"), QStringLiteral("127.0.0.1:1502"), QStringLiteral("成功"), QStringLiteral("监听中"));
    refreshAlarmRecords();
    upkun::domain::DeviceConnectionConfig config;
    m_deviceClient->connectToDevice(config);
}

void MainWindow::stopSimulator()
{
    m_deviceClient->disconnectFromDevice();
    m_simulatedServer->stop();
    m_simulatorPage->setListening(false);
    appendOperationLog(QStringLiteral("停止模拟器"), QStringLiteral("127.0.0.1:1502"), QStringLiteral("成功"), QStringLiteral("已停止监听"));
    refreshAlarmRecords();
}

QWidget* MainWindow::createStatusHeader()
{
    auto* header = new QFrame(this);
    header->setObjectName(QStringLiteral("statusHeader"));
    header->setMinimumHeight(64);
    header->setStyleSheet(QStringLiteral(
        "#statusHeader { background: #ffffff; color: #000000; border-bottom: 1px solid #d0d0d0; }"
        "QLabel { color: #000000; }"));

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
        "#navigationPanel { background: #fafafa; border-right: 1px solid #d0d0d0; }"
        "QListWidget { background: #fafafa; color: #000000; border: none; font-size: 15px; }"
        "QListWidget::item { color: #000000; padding: 14px 16px; border-bottom: 1px solid #eeeeee; }"
        "QListWidget::item:selected { background: #e8e8e8; color: #000000; border-left: 4px solid #000000; }"
        "QListWidget::item:hover { background: #f0f0f0; color: #000000; }"));

    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(0, 12, 0, 12);

    m_navigation = new QListWidget(frame);
    m_navigation->addItem(new QListWidgetItem(QStringLiteral("主监控")));
    m_navigation->addItem(new QListWidgetItem(QStringLiteral("报警记录")));
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
        "#alarmFooter { background: #ffffff; border-top: 1px solid #d0d0d0; }"
        "QLabel { color: #000000; font-weight: 600; }"));

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

void MainWindow::setupStorage()
{
    QString errorMessage;
    if (!m_databaseManager.open(QStringLiteral("data/app.sqlite3"), &errorMessage)) {
        m_alarmLabel->setText(QStringLiteral("数据库初始化失败：%1").arg(errorMessage));
        return;
    }

    m_alarmService = new upkun::services::AlarmService(&m_alarmRepository, &m_operationLogRepository, this);
    connect(m_alarmService, &upkun::services::AlarmService::recordsChanged,
        this, &MainWindow::refreshAlarmRecords);
    connect(m_alarmPage, &upkun::ui::AlarmPage::refreshRequested,
        this, &MainWindow::refreshAlarmRecords);

    appendOperationLog(QStringLiteral("启动程序"), QStringLiteral("数据库"), QStringLiteral("成功"), QStringLiteral("data/app.sqlite3"));
    refreshAlarmRecords();
}

void MainWindow::setupDeviceLink()
{
    m_simulatedServer = new upkun::simulator::SimulatedModbusServer(this);
    m_deviceClient = new upkun::device::ModbusTcpClient(this);

    connect(m_deviceClient, &upkun::device::ModbusTcpClient::connectionChanged,
        this, &MainWindow::handleConnectionChanged);
    connect(m_deviceClient, &upkun::device::ModbusTcpClient::snapshotUpdated,
        this, &MainWindow::handleSnapshotUpdated);
    connect(m_deviceClient, &upkun::device::ModbusTcpClient::commandFinished,
        this, &MainWindow::handleCommandFinished);
    connect(m_monitorPage, &upkun::ui::MonitorPage::commandRequested,
        m_deviceClient, &upkun::device::ModbusTcpClient::sendCommand);

    connect(m_simulatorPage, &upkun::ui::SimulatorPage::startSimulatorRequested,
        this, &MainWindow::startSimulator);
    connect(m_simulatorPage, &upkun::ui::SimulatorPage::stopSimulatorRequested,
        this, &MainWindow::stopSimulator);
    connect(m_simulatorPage, &upkun::ui::SimulatorPage::faultRequested,
        m_simulatedServer, &upkun::simulator::SimulatedModbusServer::triggerAlarm);
    connect(m_simulatorPage, &upkun::ui::SimulatorPage::faultRequested,
        this, [this](int alarmCode) {
            appendOperationLog(QStringLiteral("触发模拟故障"), QString::number(alarmCode), QStringLiteral("成功"), QStringLiteral("模拟器页面"));
            refreshAlarmRecords();
        });
    connect(m_simulatorPage, &upkun::ui::SimulatorPage::clearFaultRequested,
        m_simulatedServer, &upkun::simulator::SimulatedModbusServer::clearAlarm);
    connect(m_simulatorPage, &upkun::ui::SimulatorPage::clearFaultRequested,
        this, [this] {
            appendOperationLog(QStringLiteral("清除模拟故障"), QStringLiteral("模拟器"), QStringLiteral("成功"), QStringLiteral("模拟器页面"));
            refreshAlarmRecords();
        });
    connect(m_simulatedServer, &upkun::simulator::SimulatedModbusServer::listeningChanged,
        m_simulatorPage, &upkun::ui::SimulatorPage::setListening);

    startSimulator();
}

void MainWindow::appendOperationLog(const QString& action, const QString& target, const QString& result, const QString& message)
{
    m_operationLogRepository.append(action, target, result, message);
}

} // namespace upkun::app
