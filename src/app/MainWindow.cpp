#include "app/MainWindow.h"

#include "device/ModbusTcpClient.h"
#include "ui/dialogs/LoginDialog.h"
#include "ui/pages/AlarmPage.h"
#include "ui/pages/BatchPage.h"
#include "ui/pages/MonitorPage.h"
#include "ui/pages/RecipePage.h"
#include "ui/pages/SimulatorPage.h"
#include "ui/pages/TrendPage.h"

#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QPushButton>
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

int roleRank(upkun::domain::UserRole role)
{
    // 简化版权限模型：角色等级越高，可执行的操作越多。
    switch (role) {
    case upkun::domain::UserRole::Operator:
        return 1;
    case upkun::domain::UserRole::Engineer:
        return 2;
    case upkun::domain::UserRole::Administrator:
        return 3;
    }
    return 0;
}

bool recipeValuesEqual(const upkun::domain::RecipeParameters& left, const upkun::domain::RecipeParameters& right)
{
    return left.name == right.name
        && left.targetSpeed == right.targetSpeed
        && left.fillVolumeMl == right.fillVolumeMl
        && left.fillTimeMs == right.fillTimeMs
        && left.cappingTorqueCentinewtonMeter == right.cappingTorqueCentinewtonMeter
        && left.weightMinGram == right.weightMinGram
        && left.weightMaxGram == right.weightMaxGram
        && left.labelMode == right.labelMode
        && left.batchTargetCount == right.batchTargetCount
        && left.simulationQualityRate == right.simulationQualityRate;
}

QString simulatorExecutablePath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("upkun-simulator.exe"));
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
    m_batchPage = new upkun::ui::BatchPage(m_pages);
    m_alarmPage = new upkun::ui::AlarmPage(m_pages);
    m_recipePage = new upkun::ui::RecipePage(m_pages);
    m_trendPage = new upkun::ui::TrendPage(m_pages);
    m_simulatorPage = new upkun::ui::SimulatorPage(m_pages);
    m_pages->addWidget(m_monitorPage);
    m_pages->addWidget(m_batchPage);
    m_pages->addWidget(m_alarmPage);
    m_pages->addWidget(m_recipePage);
    m_pages->addWidget(m_trendPage);
    m_pages->addWidget(m_simulatorPage);
    bodyLayout->addWidget(m_pages, 1);

    rootLayout->addWidget(body, 1);
    rootLayout->addWidget(createAlarmFooter());

    setCentralWidget(root);

    connect(m_navigation, &QListWidget::currentRowChanged, this, &MainWindow::handleNavigationChanged);
    connect(&m_userSession, &upkun::services::UserSessionService::currentUserChanged,
        this, &MainWindow::handleCurrentUserChanged);
    m_navigation->setCurrentRow(0);

    setupStorage();
    setupDeviceLink();
}

MainWindow::~MainWindow()
{
    if (m_simulatorProcess != nullptr && m_simulatorProcess->state() != QProcess::NotRunning) {
        m_simulatorProcess->terminate();
        if (!m_simulatorProcess->waitForFinished(1500)) {
            m_simulatorProcess->kill();
            m_simulatorProcess->waitForFinished(1000);
        }
    }
}

void MainWindow::handleNavigationChanged(int row)
{
    if (row >= 0 && row < m_pages->count()) {
        m_pages->setCurrentIndex(row);
    }
}

void MainWindow::handleSnapshotUpdated(const upkun::domain::DeviceSnapshot& snapshot)
{
    m_latestSnapshot = snapshot;
    m_systemStateLabel->setText(QStringLiteral("系统状态：%1").arg(systemStateText(snapshot.systemState)));
    m_modeLabel->setText(QStringLiteral("当前模式：%1").arg(runModeText(snapshot.currentMode)));
    m_alarmLabel->setText(snapshot.currentAlarmCode == 0
            ? QStringLiteral("当前报警：无")
            : QStringLiteral("当前报警：%1").arg(snapshot.currentAlarmCode));

    // 每次通信快照都会同步驱动报警、监控、趋势和模拟器界面。
    if (m_alarmService != nullptr) {
        m_alarmService->processSnapshot(snapshot);
    }

    m_monitorPage->updateSnapshot(snapshot);
    m_trendPage->appendSnapshot(snapshot);
    m_simulatorPage->updateSnapshot(snapshot);
    updateActiveBatchView();

    const QDateTime now = QDateTime::currentDateTime();
    if (!m_lastTrendSampleAt.isValid() || m_lastTrendSampleAt.msecsTo(now) >= 1000) {
        // 趋势入库按 1 秒节流，避免 UI 高频刷新时把 SQLite 写爆。
        m_trendRepository.appendSample(snapshot);
        m_lastTrendSampleAt = now;
    }
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
        // 报警表的确认人和操作日志都尽量使用当前登录用户，便于追溯。
        const auto user = m_userSession.currentUser();
        if (user.has_value()) {
            m_alarmService->acknowledgeCurrentAlarm(*user);
        } else {
            m_alarmService->acknowledgeCurrentAlarm(currentUserDisplayName());
        }
    }
    refreshAlarmRecords();
}

void MainWindow::refreshAlarmRecords()
{
    if (m_alarmPage == nullptr || m_alarmService == nullptr) {
        return;
    }

    m_alarmPage->setAlarmRows(m_alarmService->alarmRows(
        m_alarmPage->stateFilter(),
        m_alarmPage->levelFilter(),
        m_alarmPage->stationFilter(),
        m_alarmPage->keywordFilter()));
    m_alarmPage->setOperationRows(m_alarmService->recentOperationRows());
}

void MainWindow::refreshBatchRecords()
{
    if (m_batchPage == nullptr) {
        return;
    }

    m_batchPage->setBatchRows(m_batchRepository.recentRows());
    updateActiveBatchView();
}

void MainWindow::refreshRecipeRecords()
{
    if (m_recipePage == nullptr) {
        return;
    }

    m_recipePage->setRecipes(m_recipeRepository.listRecipes());
    m_recipePage->setApplyRows(m_recipeRepository.recentApplyRows());
}

void MainWindow::startBatch(const QString& batchNo, int targetCount)
{
    if (!ensureRole(upkun::domain::UserRole::Operator, QStringLiteral("开始批次"))) {
        return;
    }

    const auto user = m_userSession.currentUser();
    if (!user.has_value()) {
        m_batchPage->setMessage(QStringLiteral("请先切换到登录用户"));
        return;
    }

    const auto recipe = m_recipePage->currentRecipe();
    upkun::domain::ProductionBatch batch;
    batch.batchNo = batchNo.trimmed();
    batch.recipeName = recipe.name;
    batch.operatorUserId = user->id;
    batch.operatorLoginName = user->loginName;
    batch.operatorDisplayName = user->displayName;
    batch.targetCount = targetCount > 0 ? targetCount : recipe.batchTargetCount;
    batch.startTotalCount = m_latestSnapshot.counters.total;
    batch.startGoodCount = m_latestSnapshot.counters.good;
    batch.startBadCount = m_latestSnapshot.counters.bad;

    QString errorMessage;
    if (!m_batchRepository.startBatch(&batch, &errorMessage)) {
        m_batchPage->setMessage(QStringLiteral("开始失败：%1").arg(errorMessage));
        appendOperationLog(QStringLiteral("开始批次"), batch.batchNo, QStringLiteral("失败"), errorMessage);
        refreshBatchRecords();
        return;
    }

    m_activeBatch = batch;
    m_deviceClient->sendCommand(upkun::domain::DeviceCommand::BatchStart);
    m_batchPage->setMessage(QStringLiteral("批次已开始：%1").arg(batch.batchNo));
    appendOperationLog(QStringLiteral("开始批次"), batch.batchNo, QStringLiteral("成功"),
        QStringLiteral("计划 %1 pcs，配方 %2").arg(batch.targetCount).arg(batch.recipeName));
    refreshBatchRecords();
}

void MainWindow::endBatch()
{
    if (!ensureRole(upkun::domain::UserRole::Operator, QStringLiteral("结束批次"))) {
        return;
    }
    if (!m_activeBatch.has_value()) {
        m_batchPage->setMessage(QStringLiteral("没有进行中的批次"));
        return;
    }
    if (m_latestSnapshot.systemState == upkun::domain::SystemState::Running) {
        const QString message = QStringLiteral("请先停止产线再结束批次");
        m_batchPage->setMessage(message);
        appendOperationLog(QStringLiteral("结束批次"), m_activeBatch->batchNo, QStringLiteral("拒绝"), message);
        refreshBatchRecords();
        return;
    }

    const int totalCount = activeBatchTotalCount();
    const int goodCount = activeBatchGoodCount();
    const int badCount = activeBatchBadCount();

    QString errorMessage;
    if (!m_batchRepository.finishActiveBatch(m_activeBatch->id, totalCount, goodCount, badCount, &errorMessage)) {
        m_batchPage->setMessage(QStringLiteral("结束失败：%1").arg(errorMessage));
        appendOperationLog(QStringLiteral("结束批次"), m_activeBatch->batchNo, QStringLiteral("失败"), errorMessage);
        refreshBatchRecords();
        return;
    }

    const QString finishedBatchNo = m_activeBatch->batchNo;
    m_deviceClient->sendCommand(upkun::domain::DeviceCommand::BatchEnd);
    m_activeBatch.reset();
    m_batchPage->clearActiveBatch();
    m_batchPage->setMessage(QStringLiteral("批次已结束：%1").arg(finishedBatchNo));
    appendOperationLog(QStringLiteral("结束批次"), finishedBatchNo, QStringLiteral("成功"),
        QStringLiteral("总数 %1，良品 %2，不良 %3").arg(totalCount).arg(goodCount).arg(badCount));
    refreshBatchRecords();
}

void MainWindow::loadRecipe(int recipeId)
{
    const auto recipe = m_recipeRepository.loadById(recipeId);
    if (!recipe.has_value()) {
        m_alarmLabel->setText(QStringLiteral("加载配方失败：未找到 ID %1").arg(recipeId));
        return;
    }

    m_recipePage->setRecipe(*recipe);
    m_recipePage->setMessage(QStringLiteral("已加载配方：%1 V%2").arg(recipe->name).arg(recipe->version));
    updateBatchContext();
}

void MainWindow::copyRecipe(upkun::domain::RecipeParameters recipe, QString newName)
{
    if (!ensureRole(upkun::domain::UserRole::Engineer, QStringLiteral("复制配方"))) {
        return;
    }
    if (recipe.id <= 0) {
        m_recipePage->setMessage(QStringLiteral("请先选择已保存的源配方"));
        return;
    }

    upkun::domain::RecipeParameters copied;
    QString errorMessage;
    if (!m_recipeRepository.copyRecipe(recipe.id, newName, currentUserDisplayName(), &copied, &errorMessage)) {
        m_alarmLabel->setText(QStringLiteral("复制配方失败：%1").arg(errorMessage));
        m_recipePage->setMessage(QStringLiteral("复制失败：%1").arg(errorMessage));
        appendOperationLog(QStringLiteral("复制配方"), recipe.name, QStringLiteral("失败"), errorMessage);
        refreshRecipeRecords();
        return;
    }

    m_recipePage->setRecipe(copied);
    m_recipePage->setMessage(QStringLiteral("已复制为：%1").arg(copied.name));
    m_alarmLabel->setText(QStringLiteral("配方已复制：%1").arg(copied.name));
    appendOperationLog(QStringLiteral("复制配方"), copied.name, QStringLiteral("成功"), QStringLiteral("源配方：%1").arg(recipe.name));
    refreshRecipeRecords();
    updateBatchContext();
}

void MainWindow::saveRecipe(upkun::domain::RecipeParameters recipe)
{
    // 配方影响工艺参数，先要求工程师及以上角色。
    if (!ensureRole(upkun::domain::UserRole::Engineer, QStringLiteral("保存配方"))) {
        return;
    }

    if (persistRecipe(recipe).has_value()) {
        updateBatchContext();
    }
}

void MainWindow::applyRecipe(upkun::domain::RecipeParameters recipe)
{
    // 下发前先保存，保证“界面值、数据库值、PLC 值”三者尽量一致。
    if (!ensureRole(upkun::domain::UserRole::Engineer, QStringLiteral("下发配方"))) {
        return;
    }

    auto savedRecipe = recipe.id > 0 ? m_recipeRepository.loadById(recipe.id) : std::optional<upkun::domain::RecipeParameters> {};
    if (!savedRecipe.has_value() || !recipeValuesEqual(recipe, *savedRecipe)) {
        savedRecipe = persistRecipe(recipe);
    }
    if (!savedRecipe.has_value()) {
        return;
    }

    m_deviceClient->writeRecipe(*savedRecipe);
    const auto user = m_userSession.currentUser();
    if (user.has_value()) {
        QString applyError;
        if (!m_recipeRepository.recordApply(*savedRecipe, *user, QStringLiteral("PLC/模拟器"), QStringLiteral("成功"),
                QStringLiteral("Holding Registers 40001-40010"), &applyError)) {
            m_recipePage->setMessage(QStringLiteral("下发成功，但记录失败：%1").arg(applyError));
        }
    }

    m_recipePage->setRecipe(m_recipeRepository.loadById(savedRecipe->id).value_or(*savedRecipe));
    m_alarmLabel->setText(QStringLiteral("配方已下发：%1 V%2").arg(savedRecipe->name).arg(savedRecipe->version));
    m_recipePage->setMessage(QStringLiteral("已下发：%1 V%2").arg(savedRecipe->name).arg(savedRecipe->version));
    appendOperationLog(QStringLiteral("下发配方"), savedRecipe->name, QStringLiteral("成功"), QStringLiteral("Holding Registers 40001-40010"));
    refreshRecipeRecords();
    updateBatchContext();
    refreshAlarmRecords();
}

void MainWindow::exportTrendCsv()
{
    QString errorMessage;
    const QString filePath = QStringLiteral("data/trend-export.csv");
    if (!m_trendRepository.exportCsv(filePath, &errorMessage)) {
        m_trendPage->setExportMessage(QStringLiteral("失败：%1").arg(errorMessage));
        appendOperationLog(QStringLiteral("导出趋势"), filePath, QStringLiteral("失败"), errorMessage);
        refreshAlarmRecords();
        return;
    }

    m_trendPage->setExportMessage(QStringLiteral("已导出 %1").arg(filePath));
    appendOperationLog(QStringLiteral("导出趋势"), filePath, QStringLiteral("成功"), QStringLiteral("CSV"));
    refreshAlarmRecords();
}

void MainWindow::startSimulator()
{
    const QString exePath = simulatorExecutablePath();
    if (!QFileInfo::exists(exePath)) {
        const QString message = QStringLiteral("未找到独立模拟器：%1").arg(exePath);
        m_alarmLabel->setText(message);
        appendOperationLog(QStringLiteral("启动模拟器"), QStringLiteral("127.0.0.1:1502"), QStringLiteral("失败"), message);
        refreshAlarmRecords();
        return;
    }

    if (m_simulatorProcess->state() == QProcess::NotRunning) {
        m_simulatorProcess->setProgram(exePath);
        m_simulatorProcess->setArguments({QStringLiteral("--host"), QStringLiteral("127.0.0.1"), QStringLiteral("--port"), QStringLiteral("1502")});
        m_simulatorProcess->setProcessChannelMode(QProcess::MergedChannels);
        m_simulatorProcess->start();
        if (!m_simulatorProcess->waitForStarted(3000)) {
            const QString message = m_simulatorProcess->errorString();
            m_alarmLabel->setText(QStringLiteral("模拟器启动失败：%1").arg(message));
            appendOperationLog(QStringLiteral("启动模拟器"), QStringLiteral("127.0.0.1:1502"), QStringLiteral("失败"), message);
            refreshAlarmRecords();
            return;
        }
        if (m_simulatorProcess->waitForFinished(300)) {
            const QString output = QString::fromLocal8Bit(m_simulatorProcess->readAll()).trimmed();
            const QString message = output.isEmpty() ? m_simulatorProcess->errorString() : output;
            m_alarmLabel->setText(QStringLiteral("模拟器启动失败：%1").arg(message));
            appendOperationLog(QStringLiteral("启动模拟器"), QStringLiteral("127.0.0.1:1502"), QStringLiteral("失败"), message);
            refreshAlarmRecords();
            return;
        }
    }

    m_simulatorPage->setListening(true);
    appendOperationLog(QStringLiteral("启动模拟器"), QStringLiteral("127.0.0.1:1502"), QStringLiteral("成功"), QStringLiteral("独立进程"));
    refreshAlarmRecords();
    upkun::domain::DeviceConnectionConfig config;
    m_deviceClient->connectToDevice(config);
}

void MainWindow::stopSimulator()
{
    m_deviceClient->disconnectFromDevice();
    if (m_simulatorProcess->state() != QProcess::NotRunning) {
        m_simulatorProcess->terminate();
        if (!m_simulatorProcess->waitForFinished(1500)) {
            m_simulatorProcess->kill();
            m_simulatorProcess->waitForFinished(1000);
        }
    }
    m_simulatorPage->setListening(false);
    appendOperationLog(QStringLiteral("停止模拟器"), QStringLiteral("127.0.0.1:1502"), QStringLiteral("成功"), QStringLiteral("已停止监听"));
    refreshAlarmRecords();
}

void MainWindow::switchUser()
{
    // 快速切换面向现场共用工控机：不退出程序，只切换当前操作身份。
    const auto users = m_userRepository.enabledUsers();
    upkun::ui::LoginDialog dialog(users, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString errorMessage;
    auto user = m_userRepository.authenticate(dialog.loginName(), dialog.password(), &errorMessage);
    if (!user.has_value()) {
        m_alarmLabel->setText(QStringLiteral("切换用户失败：%1").arg(errorMessage));
        appendOperationLog(QStringLiteral("切换用户"), dialog.loginName(), QStringLiteral("失败"), errorMessage);
        refreshAlarmRecords();
        return;
    }

    const QString previousUser = currentUserDisplayName();
    m_userSession.setCurrentUser(*user);
    m_alarmLabel->setText(QStringLiteral("当前用户已切换：%1").arg(user->displayName));
    appendOperationLog(QStringLiteral("切换用户"), user->loginName, QStringLiteral("成功"),
        QStringLiteral("%1 -> %2").arg(previousUser, user->displayName));
    refreshAlarmRecords();
}

void MainWindow::handleCurrentUserChanged()
{
    const auto user = m_userSession.currentUser();
    if (user.has_value()) {
        m_userLabel->setText(QStringLiteral("当前用户：%1（%2）")
                .arg(user->displayName, upkun::domain::userRoleText(user->role)));
        updateBatchContext();
        return;
    }

    m_userLabel->setText(QStringLiteral("当前用户：未登录"));
    updateBatchContext();
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
    m_switchUserButton = new QPushButton(QStringLiteral("切换用户"), header);
    m_switchUserButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #f5f5f5; color: #000000; border: 1px solid #a0a0a0; min-height: 30px; padding: 0 14px; }"
        "QPushButton:hover { background: #eeeeee; }"
        "QPushButton:pressed { background: #dddddd; }"));
    connect(m_switchUserButton, &QPushButton::clicked, this, &MainWindow::switchUser);

    layout->addWidget(m_systemStateLabel);
    layout->addWidget(m_modeLabel);
    layout->addWidget(m_connectionLabel);
    layout->addStretch(1);
    layout->addWidget(m_userLabel);
    layout->addWidget(m_switchUserButton);

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
    m_navigation->addItem(new QListWidgetItem(QStringLiteral("批次管理")));
    m_navigation->addItem(new QListWidgetItem(QStringLiteral("报警记录")));
    m_navigation->addItem(new QListWidgetItem(QStringLiteral("参数/配方")));
    m_navigation->addItem(new QListWidgetItem(QStringLiteral("趋势曲线")));
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

    QString userError;
    if (!m_userRepository.ensureDefaultUsers(&userError)) {
        m_alarmLabel->setText(QStringLiteral("默认用户初始化失败：%1").arg(userError));
    } else {
        loginDefaultOperator();
    }

    QString recipeError;
    if (!m_recipeRepository.ensureDefaultRecipe(&recipeError)) {
        m_alarmLabel->setText(QStringLiteral("默认配方初始化失败：%1").arg(recipeError));
    } else if (m_recipePage != nullptr) {
        m_recipePage->setRecipe(m_recipeRepository.loadDefault());
    }
    refreshRecipeRecords();
    updateBatchContext();
    loadActiveBatch();

    m_alarmService = new upkun::services::AlarmService(&m_alarmRepository, &m_operationLogRepository, this);
    connect(m_alarmService, &upkun::services::AlarmService::recordsChanged,
        this, &MainWindow::refreshAlarmRecords);
    connect(m_alarmPage, &upkun::ui::AlarmPage::refreshRequested,
        this, &MainWindow::refreshAlarmRecords);
    connect(m_batchPage, &upkun::ui::BatchPage::startBatchRequested,
        this, &MainWindow::startBatch);
    connect(m_batchPage, &upkun::ui::BatchPage::endBatchRequested,
        this, &MainWindow::endBatch);
    connect(m_batchPage, &upkun::ui::BatchPage::refreshRequested,
        this, &MainWindow::refreshBatchRecords);
    connect(m_recipePage, &upkun::ui::RecipePage::saveRequested,
        this, &MainWindow::saveRecipe);
    connect(m_recipePage, &upkun::ui::RecipePage::applyRequested,
        this, &MainWindow::applyRecipe);
    connect(m_recipePage, &upkun::ui::RecipePage::copyRequested,
        this, &MainWindow::copyRecipe);
    connect(m_recipePage, &upkun::ui::RecipePage::recipeSelected,
        this, &MainWindow::loadRecipe);
    connect(m_recipePage, &upkun::ui::RecipePage::refreshRequested,
        this, &MainWindow::refreshRecipeRecords);
    connect(m_trendPage, &upkun::ui::TrendPage::exportRequested,
        this, &MainWindow::exportTrendCsv);

    appendOperationLog(QStringLiteral("启动程序"), QStringLiteral("数据库"), QStringLiteral("成功"), QStringLiteral("data/app.sqlite3"));
    refreshAlarmRecords();
    refreshBatchRecords();
}

void MainWindow::setupDeviceLink()
{
    // M14 后模拟器作为独立进程运行，上位机只通过 Modbus TCP 协议连接它。
    m_simulatorProcess = new QProcess(this);
    m_deviceClient = new upkun::device::ModbusTcpClient(this);

    connect(m_deviceClient, &upkun::device::ModbusTcpClient::connectionChanged,
        this, &MainWindow::handleConnectionChanged);
    connect(m_deviceClient, &upkun::device::ModbusTcpClient::snapshotUpdated,
        this, &MainWindow::handleSnapshotUpdated);
    connect(m_deviceClient, &upkun::device::ModbusTcpClient::commandFinished,
        this, &MainWindow::handleCommandFinished);
    connect(m_monitorPage, &upkun::ui::MonitorPage::commandRequested,
        m_deviceClient, &upkun::device::ModbusTcpClient::sendCommand);
    connect(m_simulatorProcess, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitCode)
        Q_UNUSED(exitStatus)
        m_simulatorPage->setListening(false);
    });

    connect(m_simulatorPage, &upkun::ui::SimulatorPage::startSimulatorRequested,
        this, &MainWindow::startSimulator);
    connect(m_simulatorPage, &upkun::ui::SimulatorPage::stopSimulatorRequested,
        this, &MainWindow::stopSimulator);
    connect(m_simulatorPage, &upkun::ui::SimulatorPage::faultRequested,
        m_deviceClient, &upkun::device::ModbusTcpClient::injectFault);
    connect(m_simulatorPage, &upkun::ui::SimulatorPage::faultRequested,
        this, [this](int alarmCode) {
            appendOperationLog(QStringLiteral("触发模拟故障"), QString::number(alarmCode), QStringLiteral("成功"), QStringLiteral("Modbus TCP"));
            refreshAlarmRecords();
        });
    connect(m_simulatorPage, &upkun::ui::SimulatorPage::clearFaultRequested,
        m_deviceClient, [this] {
            m_deviceClient->sendCommand(upkun::domain::DeviceCommand::Reset);
        });
    connect(m_simulatorPage, &upkun::ui::SimulatorPage::clearFaultRequested,
        this, [this] {
            appendOperationLog(QStringLiteral("清除模拟故障"), QStringLiteral("模拟器"), QStringLiteral("成功"), QStringLiteral("Modbus TCP"));
            refreshAlarmRecords();
        });

    startSimulator();
}

void MainWindow::loginDefaultOperator()
{
    // 自动登录避免无界面烟测被登录弹窗阻塞；真实交付版应改成强制登录。
    auto user = m_userRepository.findEnabledByLoginName(QStringLiteral("op001"));
    if (!user.has_value()) {
        m_userSession.clear();
        return;
    }

    QString errorMessage;
    m_userRepository.updateLastLogin(user->id, &errorMessage);
    m_userSession.setCurrentUser(*user);
}

void MainWindow::updateBatchContext()
{
    if (m_batchPage == nullptr || m_recipePage == nullptr) {
        return;
    }

    const auto recipe = m_recipePage->currentRecipe();
    m_batchPage->setCurrentContext(recipe.name, currentUserDisplayName(), recipe.batchTargetCount);
}

void MainWindow::updateActiveBatchView()
{
    if (m_batchPage == nullptr) {
        return;
    }

    if (m_activeBatch.has_value()) {
        m_batchPage->setActiveBatch(*m_activeBatch, m_latestSnapshot);
        return;
    }

    m_batchPage->clearActiveBatch();
}

void MainWindow::loadActiveBatch()
{
    m_activeBatch = m_batchRepository.activeBatch();
    updateActiveBatchView();
}

int MainWindow::activeBatchTotalCount() const
{
    if (!m_activeBatch.has_value()) {
        return 0;
    }
    return qMax(0, m_latestSnapshot.counters.batch);
}

int MainWindow::activeBatchGoodCount() const
{
    if (!m_activeBatch.has_value()) {
        return 0;
    }
    return qMax(0, m_latestSnapshot.counters.good - m_activeBatch->startGoodCount);
}

int MainWindow::activeBatchBadCount() const
{
    if (!m_activeBatch.has_value()) {
        return 0;
    }
    return qMax(0, m_latestSnapshot.counters.bad - m_activeBatch->startBadCount);
}

QString MainWindow::currentUserDisplayName() const
{
    const auto user = m_userSession.currentUser();
    return user.has_value() ? user->displayName : QStringLiteral("未登录");
}

bool MainWindow::ensureRole(upkun::domain::UserRole minimumRole, const QString& action)
{
    // 权限失败也写操作日志，方便之后分析“谁尝试做了什么”。
    const auto user = m_userSession.currentUser();
    if (!user.has_value()) {
        const QString message = QStringLiteral("请先切换到有权限的用户");
        m_alarmLabel->setText(QStringLiteral("%1被拒绝：%2").arg(action, message));
        appendOperationLog(action, QStringLiteral("权限"), QStringLiteral("拒绝"), message);
        refreshAlarmRecords();
        return false;
    }

    if (roleRank(user->role) < roleRank(minimumRole)) {
        const QString message = QStringLiteral("当前角色为%1，至少需要%2")
                .arg(upkun::domain::userRoleText(user->role), upkun::domain::userRoleText(minimumRole));
        m_alarmLabel->setText(QStringLiteral("%1被拒绝：%2").arg(action, message));
        appendOperationLog(action, QStringLiteral("权限"), QStringLiteral("拒绝"), message);
        refreshAlarmRecords();
        return false;
    }

    return true;
}

std::optional<upkun::domain::RecipeParameters> MainWindow::persistRecipe(const upkun::domain::RecipeParameters& recipe)
{
    // 保存配方被 save/apply 两个入口共用，避免下发时重复绕过错误处理。
    QString errorMessage;
    if (!m_recipeRepository.save(recipe, currentUserDisplayName(), &errorMessage)) {
        m_alarmLabel->setText(QStringLiteral("保存配方失败：%1").arg(errorMessage));
        m_recipePage->setMessage(QStringLiteral("保存失败：%1").arg(errorMessage));
        appendOperationLog(QStringLiteral("保存配方"), recipe.name, QStringLiteral("失败"), errorMessage);
        refreshAlarmRecords();
        return std::nullopt;
    }

    const auto savedRecipe = m_recipeRepository.loadByName(recipe.name);
    if (savedRecipe.has_value()) {
        m_recipePage->setRecipe(*savedRecipe);
        m_recipePage->setMessage(QStringLiteral("已保存：%1 V%2").arg(savedRecipe->name).arg(savedRecipe->version));
    }

    const int version = savedRecipe.has_value() ? savedRecipe->version : recipe.version;
    m_alarmLabel->setText(QStringLiteral("配方已保存：%1 V%2").arg(recipe.name).arg(version));
    appendOperationLog(QStringLiteral("保存配方"), recipe.name, QStringLiteral("成功"), QStringLiteral("SQLite V%1").arg(version));
    refreshRecipeRecords();
    refreshAlarmRecords();
    return savedRecipe;
}

void MainWindow::appendOperationLog(const QString& action, const QString& target, const QString& result, const QString& message)
{
    // 所有界面操作统一走这里，减少遗漏当前用户审计的机会。
    const auto user = m_userSession.currentUser();
    if (user.has_value()) {
        m_operationLogRepository.append(*user, action, target, result, message);
        return;
    }

    m_operationLogRepository.append(action, target, result, message);
}

} // namespace upkun::app
