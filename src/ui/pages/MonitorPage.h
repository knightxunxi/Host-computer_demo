#pragma once

#include "domain/DeviceTypes.h"

#include <QLabel>
#include <QPushButton>
#include <QVector>
#include <QWidget>

class QGridLayout;

namespace upkun::ui {

class MonitorPage final : public QWidget {
    Q_OBJECT

public:
    explicit MonitorPage(QWidget* parent = nullptr);

public slots:
    void updateSnapshot(const upkun::domain::DeviceSnapshot& snapshot);

signals:
    void commandRequested(upkun::domain::DeviceCommand command);

private:
    QWidget* createStationCard(const QString& title, const QString& state);
    QWidget* createMetricCard(const QString& title, const QString& value, QLabel** valueLabel);
    void addControlButtons(QGridLayout* layout);

    QVector<QLabel*> m_stationStateLabels;
    QLabel* m_totalValueLabel = nullptr;
    QLabel* m_goodValueLabel = nullptr;
    QLabel* m_badValueLabel = nullptr;
    QLabel* m_speedValueLabel = nullptr;
    QLabel* m_batchValueLabel = nullptr;
    QLabel* m_alarmValueLabel = nullptr;
    QLabel* m_fillValueLabel = nullptr;
    QLabel* m_weightValueLabel = nullptr;
    QPushButton* m_startButton = nullptr;
    QPushButton* m_stopButton = nullptr;
    QPushButton* m_resetButton = nullptr;
    QPushButton* m_alarmAckButton = nullptr;
};

} // namespace upkun::ui
