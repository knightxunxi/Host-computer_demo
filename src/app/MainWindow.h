#pragma once

#include "domain/DeviceTypes.h"

#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QStackedWidget>

namespace upkun::device {
class ModbusTcpClient;
}

namespace upkun::simulator {
class SimulatedModbusServer;
}

namespace upkun::ui {
class MonitorPage;
class SimulatorPage;
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
    void startSimulator();
    void stopSimulator();

private:
    QWidget* createStatusHeader();
    QWidget* createNavigation();
    QWidget* createAlarmFooter();
    QLabel* makeStatusLabel(const QString& title, const QString& value);
    void setupDeviceLink();

    QLabel* m_systemStateLabel = nullptr;
    QLabel* m_modeLabel = nullptr;
    QLabel* m_connectionLabel = nullptr;
    QLabel* m_userLabel = nullptr;
    QLabel* m_alarmLabel = nullptr;
    QListWidget* m_navigation = nullptr;
    QStackedWidget* m_pages = nullptr;
    upkun::ui::MonitorPage* m_monitorPage = nullptr;
    upkun::ui::SimulatorPage* m_simulatorPage = nullptr;
    upkun::simulator::SimulatedModbusServer* m_simulatedServer = nullptr;
    upkun::device::ModbusTcpClient* m_deviceClient = nullptr;
};

} // namespace upkun::app
