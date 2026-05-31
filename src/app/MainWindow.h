#pragma once

#include "domain/DeviceTypes.h"
#include "domain/User.h"
#include "services/AlarmService.h"
#include "services/UserSessionService.h"
#include "storage/AlarmRepository.h"
#include "storage/DatabaseManager.h"
#include "storage/OperationLogRepository.h"
#include "storage/RecipeRepository.h"
#include "storage/TrendRepository.h"
#include "storage/UserRepository.h"

#include <QDateTime>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>

namespace upkun::device {
class ModbusTcpClient;
}

namespace upkun::simulator {
class SimulatedModbusServer;
}

namespace upkun::ui {
class AlarmPage;
class MonitorPage;
class RecipePage;
class SimulatorPage;
class TrendPage;
}

namespace upkun::app {

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void handleNavigationChanged(int row);
    void handleSnapshotUpdated(const upkun::domain::DeviceSnapshot& snapshot);
    void handleConnectionChanged(upkun::domain::ConnectionState state);
    void handleCommandFinished(upkun::domain::DeviceCommand command, bool ok, const QString& message);
    void refreshAlarmRecords();
    void saveRecipe(upkun::domain::RecipeParameters recipe);
    void applyRecipe(upkun::domain::RecipeParameters recipe);
    void exportTrendCsv();
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
    void loginDefaultOperator();
    QString currentUserDisplayName() const;
    bool ensureRole(upkun::domain::UserRole minimumRole, const QString& action);
    bool persistRecipe(const upkun::domain::RecipeParameters& recipe);
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
    upkun::ui::AlarmPage* m_alarmPage = nullptr;
    upkun::ui::RecipePage* m_recipePage = nullptr;
    upkun::ui::TrendPage* m_trendPage = nullptr;
    upkun::ui::SimulatorPage* m_simulatorPage = nullptr;
    upkun::simulator::SimulatedModbusServer* m_simulatedServer = nullptr;
    upkun::device::ModbusTcpClient* m_deviceClient = nullptr;
    upkun::storage::DatabaseManager m_databaseManager;
    upkun::storage::AlarmRepository m_alarmRepository;
    upkun::storage::OperationLogRepository m_operationLogRepository;
    upkun::storage::UserRepository m_userRepository;
    upkun::storage::RecipeRepository m_recipeRepository;
    upkun::storage::TrendRepository m_trendRepository;
    upkun::services::UserSessionService m_userSession;
    upkun::services::AlarmService* m_alarmService = nullptr;
    QDateTime m_lastTrendSampleAt;
};

} // namespace upkun::app
