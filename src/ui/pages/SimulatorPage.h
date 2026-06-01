#pragma once

#include "domain/DeviceTypes.h"

#include <QLabel>
#include <QWidget>

namespace upkun::ui {

class SimulatorPage final : public QWidget {
    Q_OBJECT

public:
    explicit SimulatorPage(QWidget* parent = nullptr);

public slots:
    void updateSnapshot(const upkun::domain::DeviceSnapshot& snapshot);
    void setEndpoint(const QString& endpoint);
    void setListening(bool listening);

signals:
    void startSimulatorRequested();
    void stopSimulatorRequested();
    void faultRequested(int alarmCode);
    void clearFaultRequested();

private:
    QLabel* m_listeningLabel = nullptr;
    QLabel* m_alarmLabel = nullptr;
    QLabel* m_countLabel = nullptr;
    QLabel* m_processLabel = nullptr;
    QLabel* m_sensorLabel = nullptr;
    QLabel* m_actuatorLabel = nullptr;
    QLabel* m_qualityLabel = nullptr;
    QString m_endpoint = QStringLiteral("127.0.0.1:1502");
};

} // namespace upkun::ui
