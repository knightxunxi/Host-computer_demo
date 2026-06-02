#pragma once

#include "domain/Batch.h"
#include "domain/DeviceTypes.h"
#include "domain/User.h"
#include "infrastructure/AppConfig.h"
#include "services/AlarmService.h"
#include "services/LineControlService.h"
#include "services/UserSessionService.h"
#include "storage/AlarmRepository.h"
#include "storage/BatchRepository.h"
#include "storage/DatabaseManager.h"
#include "storage/OperationLogRepository.h"
#include "storage/RecipeRepository.h"
#include "storage/ReportRepository.h"
#include "storage/TrendRepository.h"
#include "storage/UserRepository.h"

#include <QDateTime>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QProcess>
#include <QPushButton>
#include <QStackedWidget>
#include <optional>

namespace upkun::device {
class IDeviceClient;
}

namespace upkun::ui {
class AlarmPage;
class BatchPage;
class DiagnosticsPage;
class MonitorPage;
class RecipePage;
class ReportPage;
class SimulatorPage;
class SettingsPage;
class StationDetailPage;
class TrendPage;
}

namespace upkun::app {

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void handleNavigationChanged(int row);
    void handleSnapshotUpdated(const upkun::domain::DeviceSnapshot& snapshot);
    void handleConnectionChanged(upkun::domain::ConnectionState state);
    void handleDiagnosticsUpdated(const upkun::domain::CommunicationDiagnostics& diagnostics);
    void handleCommandFinished(upkun::domain::DeviceCommand command, bool ok, const QString& message);
    void refreshAlarmRecords();
    void refreshBatchRecords();
    void refreshRecipeRecords();
    void startBatch(const QString& batchNo, int targetCount);
    void endBatch();
    void loadRecipe(int recipeId);
    void copyRecipe(upkun::domain::RecipeParameters recipe, QString newName);
    void saveRecipe(upkun::domain::RecipeParameters recipe);
    void applyRecipe(upkun::domain::RecipeParameters recipe);
    void exportTrendCsv();
    void handleLineCommandRequested(upkun::domain::DeviceCommand command);
    void handleConfigSaveRequested(upkun::infrastructure::AppConfig config, bool reconnect);
    void connectDevice();
    void disconnectDevice();
    void refreshReports();
    void exportReportCsv();
    void startSimulator();
    void stopSimulator();
    void switchUser();
    void handleCurrentUserChanged();

private:
    QWidget* createStatusHeader();
    QWidget* createNavigation();
    QWidget* createAlarmFooter();
    QLabel* makeStatusLabel(const QString& title, const QString& value);
    void setupStorage();
    void setupDeviceLink();
    void configureDeviceClient();
    void loginDefaultOperator();
    void updateBatchContext();
    void updateActiveBatchView();
    void loadActiveBatch();
    int activeBatchTotalCount() const;
    int activeBatchGoodCount() const;
    int activeBatchBadCount() const;
    QString currentUserDisplayName() const;
    bool ensureRole(upkun::domain::UserRole minimumRole, const QString& action);
    std::optional<upkun::domain::RecipeParameters> persistRecipe(const upkun::domain::RecipeParameters& recipe);
    void appendOperationLog(const QString& action, const QString& target, const QString& result, const QString& message);

    QLabel* m_systemStateLabel = nullptr;
    QLabel* m_modeLabel = nullptr;
    QLabel* m_connectionLabel = nullptr;
    QLabel* m_userLabel = nullptr;
    QPushButton* m_switchUserButton = nullptr;
    QLabel* m_alarmLabel = nullptr;
    QListWidget* m_navigation = nullptr;
    QStackedWidget* m_pages = nullptr;
    upkun::ui::MonitorPage* m_monitorPage = nullptr;
    upkun::ui::BatchPage* m_batchPage = nullptr;
    upkun::ui::AlarmPage* m_alarmPage = nullptr;
    upkun::ui::RecipePage* m_recipePage = nullptr;
    upkun::ui::TrendPage* m_trendPage = nullptr;
    upkun::ui::StationDetailPage* m_stationDetailPage = nullptr;
    upkun::ui::ReportPage* m_reportPage = nullptr;
    upkun::ui::SimulatorPage* m_simulatorPage = nullptr;
    upkun::ui::DiagnosticsPage* m_diagnosticsPage = nullptr;
    upkun::ui::SettingsPage* m_settingsPage = nullptr;
    QProcess* m_simulatorProcess = nullptr;
    upkun::device::IDeviceClient* m_deviceClient = nullptr;
    upkun::infrastructure::AppConfig m_config;
    upkun::storage::DatabaseManager m_databaseManager;
    upkun::storage::AlarmRepository m_alarmRepository;
    upkun::storage::BatchRepository m_batchRepository;
    upkun::storage::OperationLogRepository m_operationLogRepository;
    upkun::storage::UserRepository m_userRepository;
    upkun::storage::RecipeRepository m_recipeRepository;
    upkun::storage::TrendRepository m_trendRepository;
    upkun::storage::ReportRepository m_reportRepository;
    upkun::services::UserSessionService m_userSession;
    upkun::services::LineControlService m_lineControlService;
    upkun::services::AlarmService* m_alarmService = nullptr;
    QDateTime m_lastTrendSampleAt;
    upkun::domain::DeviceSnapshot m_latestSnapshot;
    std::optional<upkun::domain::ProductionBatch> m_activeBatch;
};

} // namespace upkun::app
