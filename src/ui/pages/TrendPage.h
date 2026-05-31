#pragma once

#include "domain/DeviceTypes.h"

#include <QChartView>
#include <QLabel>
#include <QLineSeries>
#include <QValueAxis>
#include <QWidget>

namespace upkun::ui {

class TrendPage final : public QWidget {
    Q_OBJECT

public:
    explicit TrendPage(QWidget* parent = nullptr);

public slots:
    void appendSnapshot(const upkun::domain::DeviceSnapshot& snapshot);
    void setExportMessage(const QString& message);

signals:
    void exportRequested();

private:
    void appendPoint(QLineSeries* series, double value);
    void trimSeries(QLineSeries* series) const;
    void refreshAxes();

    QLineSeries* m_speedSeries = nullptr;
    QLineSeries* m_fillSeries = nullptr;
    QLineSeries* m_weightSeries = nullptr;
    QValueAxis* m_axisX = nullptr;
    QValueAxis* m_axisY = nullptr;
    QLabel* m_exportLabel = nullptr;
    int m_index = 0;
    int m_maxPoints = 120;
};

} // namespace upkun::ui
