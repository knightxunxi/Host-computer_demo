#pragma once

#include <QWidget>

class QGridLayout;

namespace upkun::ui {

class MonitorPage final : public QWidget {
    Q_OBJECT

public:
    explicit MonitorPage(QWidget* parent = nullptr);

private:
    QWidget* createStationCard(const QString& title, const QString& state);
    QWidget* createMetricCard(const QString& title, const QString& value);
    void addControlButtons(QGridLayout* layout);
};

} // namespace upkun::ui
