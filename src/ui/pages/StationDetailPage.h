#pragma once

#include "domain/DeviceTypes.h"

#include <QLabel>
#include <QTableWidget>
#include <QWidget>

namespace upkun::ui {

class StationDetailPage final : public QWidget {
    Q_OBJECT

public:
    explicit StationDetailPage(QWidget* parent = nullptr);

public slots:
    void updateSnapshot(const upkun::domain::DeviceSnapshot& snapshot);
    void setMessage(const QString& message);

signals:
    void commandRequested(upkun::domain::DeviceCommand command);

private:
    void setRow(int row, const QString& station, const QString& sensor, const QString& actuator, const QString& status);

    QTableWidget* m_stationTable = nullptr;
    QLabel* m_messageLabel = nullptr;
};

} // namespace upkun::ui
