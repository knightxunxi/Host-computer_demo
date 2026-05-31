#include "ui/pages/TrendPage.h"

#include <QChart>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

namespace upkun::ui {

TrendPage::TrendPage(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(QStringLiteral(
        "QWidget { background: #ffffff; color: #000000; }"
        "QPushButton { background: #f5f5f5; color: #000000; border: 1px solid #a0a0a0; min-height: 30px; padding: 0 14px; }"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    auto* title = new QLabel(QStringLiteral("趋势曲线"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    auto* exportButton = new QPushButton(QStringLiteral("导出趋势CSV"), this);
    connect(exportButton, &QPushButton::clicked, this, &TrendPage::exportRequested);
    rootLayout->addWidget(exportButton, 0, Qt::AlignLeft);

    m_exportLabel = new QLabel(QStringLiteral("导出状态：未导出"), this);
    rootLayout->addWidget(m_exportLabel);

    m_speedSeries = new QLineSeries(this);
    m_speedSeries->setName(QStringLiteral("速度 pcs/min"));
    m_fillSeries = new QLineSeries(this);
    m_fillSeries->setName(QStringLiteral("灌装量 ml"));
    m_weightSeries = new QLineSeries(this);
    m_weightSeries->setName(QStringLiteral("重量 g"));

    auto* chart = new QChart();
    chart->setTitle(QStringLiteral("最近过程数据"));
    chart->setBackgroundBrush(Qt::white);
    chart->addSeries(m_speedSeries);
    chart->addSeries(m_fillSeries);
    chart->addSeries(m_weightSeries);

    m_axisX = new QValueAxis(this);
    m_axisX->setTitleText(QStringLiteral("采样点"));
    m_axisX->setRange(0, m_maxPoints);
    m_axisY = new QValueAxis(this);
    m_axisY->setTitleText(QStringLiteral("数值"));
    m_axisY->setRange(0, 600);

    chart->addAxis(m_axisX, Qt::AlignBottom);
    chart->addAxis(m_axisY, Qt::AlignLeft);
    m_speedSeries->attachAxis(m_axisX);
    m_speedSeries->attachAxis(m_axisY);
    m_fillSeries->attachAxis(m_axisX);
    m_fillSeries->attachAxis(m_axisY);
    m_weightSeries->attachAxis(m_axisX);
    m_weightSeries->attachAxis(m_axisY);

    auto* chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(420);
    rootLayout->addWidget(chartView, 1);
}

void TrendPage::appendSnapshot(const upkun::domain::DeviceSnapshot& snapshot)
{
    appendPoint(m_speedSeries, snapshot.counters.speed);
    appendPoint(m_fillSeries, snapshot.processValues.fillVolumeMl);
    appendPoint(m_weightSeries, snapshot.processValues.weightGram);
    ++m_index;
    refreshAxes();
}

void TrendPage::setExportMessage(const QString& message)
{
    m_exportLabel->setText(QStringLiteral("导出状态：%1").arg(message));
}

void TrendPage::appendPoint(QLineSeries* series, double value)
{
    series->append(m_index, value);
    trimSeries(series);
}

void TrendPage::trimSeries(QLineSeries* series) const
{
    while (series->count() > m_maxPoints) {
        series->remove(0);
    }
}

void TrendPage::refreshAxes()
{
    const int minX = qMax(0, m_index - m_maxPoints);
    m_axisX->setRange(minX, qMax(m_maxPoints, m_index));
}

} // namespace upkun::ui
