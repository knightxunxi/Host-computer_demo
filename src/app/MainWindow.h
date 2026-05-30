#pragma once

#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QStackedWidget>

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

private:
    QWidget* createStatusHeader();
    QWidget* createNavigation();
    QWidget* createAlarmFooter();
    QLabel* makeStatusLabel(const QString& title, const QString& value);

    QLabel* m_systemStateLabel = nullptr;
    QLabel* m_modeLabel = nullptr;
    QLabel* m_connectionLabel = nullptr;
    QLabel* m_userLabel = nullptr;
    QLabel* m_alarmLabel = nullptr;
    QListWidget* m_navigation = nullptr;
    QStackedWidget* m_pages = nullptr;
};

} // namespace upkun::app
