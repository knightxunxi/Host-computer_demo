#include "ui/pages/DiagnosticsPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace {

QString connectionText(upkun::domain::ConnectionState state)
{
    switch (state) {
    case upkun::domain::ConnectionState::Disconnected:
        return QStringLiteral("断开");
    case upkun::domain::ConnectionState::Connecting:
        return QStringLiteral("连接中");
    case upkun::domain::ConnectionState::Connected:
        return QStringLiteral("已连接");
    case upkun::domain::ConnectionState::Reconnecting:
        return QStringLiteral("重连中");
    case upkun::domain::ConnectionState::Error:
        return QStringLiteral("错误");
    }
    return QStringLiteral("未知");
}

QString timeText(const QDateTime& time)
{
    return time.isValid() ? time.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz")) : QStringLiteral("-");
}

} // namespace

namespace upkun::ui {

DiagnosticsPage::DiagnosticsPage(QWidget* parent)
    : QWidget(parent)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(18);
    setStyleSheet(QStringLiteral("QWidget { background: #ffffff; color: #000000; }"));

    auto* title = new QLabel(QStringLiteral("通信诊断"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    auto* grid = new QGridLayout();
    grid->setSpacing(12);
    grid->addWidget(createMetricCard(QStringLiteral("连接状态"), QStringLiteral("断开"), &m_stateValueLabel), 0, 0);
    grid->addWidget(createMetricCard(QStringLiteral("目标端点"), QStringLiteral("-"), &m_endpointValueLabel), 0, 1);
    grid->addWidget(createMetricCard(QStringLiteral("通信质量"), QStringLiteral("100%"), &m_qualityValueLabel), 0, 2);
    grid->addWidget(createMetricCard(QStringLiteral("挂起请求"), QStringLiteral("0"), &m_pendingValueLabel), 0, 3);
    grid->addWidget(createMetricCard(QStringLiteral("请求总数"), QStringLiteral("0"), &m_requestValueLabel), 1, 0);
    grid->addWidget(createMetricCard(QStringLiteral("响应总数"), QStringLiteral("0"), &m_responseValueLabel), 1, 1);
    grid->addWidget(createMetricCard(QStringLiteral("超时次数"), QStringLiteral("0"), &m_timeoutValueLabel), 1, 2);
    grid->addWidget(createMetricCard(QStringLiteral("错误次数"), QStringLiteral("0"), &m_errorValueLabel), 1, 3);
    grid->addWidget(createMetricCard(QStringLiteral("重连次数"), QStringLiteral("0"), &m_reconnectValueLabel), 2, 0);
    grid->addWidget(createMetricCard(QStringLiteral("连续超时"), QStringLiteral("0"), &m_consecutiveTimeoutValueLabel), 2, 1);
    grid->addWidget(createMetricCard(QStringLiteral("最近耗时"), QStringLiteral("0 ms"), &m_roundTripValueLabel), 2, 2);
    grid->addWidget(createMetricCard(QStringLiteral("最近错误"), QStringLiteral("-"), &m_lastErrorValueLabel), 2, 3);
    grid->addWidget(createMetricCard(QStringLiteral("最近请求"), QStringLiteral("-"), &m_lastRequestValueLabel), 3, 0, 1, 2);
    grid->addWidget(createMetricCard(QStringLiteral("最近响应"), QStringLiteral("-"), &m_lastResponseValueLabel), 3, 2, 1, 2);
    rootLayout->addLayout(grid);

    auto* hint = new QLabel(QStringLiteral("通信质量按已完成请求中的成功响应比例计算；连续超时达到阈值后，客户端会主动断开并进入重连。"), this);
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("color: #000000;"));
    rootLayout->addWidget(hint);
    rootLayout->addStretch(1);
}

void DiagnosticsPage::updateDiagnostics(const upkun::domain::CommunicationDiagnostics& diagnostics)
{
    setValue(m_stateValueLabel, connectionText(diagnostics.state));
    setValue(m_endpointValueLabel, diagnostics.endpoint.isEmpty() ? QStringLiteral("-") : diagnostics.endpoint);
    setValue(m_qualityValueLabel, QStringLiteral("%1%").arg(diagnostics.qualityPercent));
    setValue(m_pendingValueLabel, QString::number(diagnostics.pendingRequests));
    setValue(m_requestValueLabel, QString::number(diagnostics.totalRequests));
    setValue(m_responseValueLabel, QString::number(diagnostics.totalResponses));
    setValue(m_timeoutValueLabel, QString::number(diagnostics.timeoutCount));
    setValue(m_errorValueLabel, QString::number(diagnostics.errorCount));
    setValue(m_reconnectValueLabel, QString::number(diagnostics.reconnectCount));
    setValue(m_consecutiveTimeoutValueLabel, QString::number(diagnostics.consecutiveTimeouts));
    setValue(m_roundTripValueLabel, QStringLiteral("%1 ms").arg(diagnostics.lastRoundTripMs));
    setValue(m_lastRequestValueLabel, timeText(diagnostics.lastRequestAt));
    setValue(m_lastResponseValueLabel, timeText(diagnostics.lastResponseAt));
    setValue(m_lastErrorValueLabel, diagnostics.lastError.isEmpty() ? QStringLiteral("-") : diagnostics.lastError);
}

QWidget* DiagnosticsPage::createMetricCard(const QString& title, const QString& value, QLabel** valueLabel)
{
    auto* card = new QFrame(this);
    card->setObjectName(QStringLiteral("diagnosticCard"));
    card->setMinimumHeight(92);
    card->setStyleSheet(QStringLiteral(
        "#diagnosticCard { background: #ffffff; border: 1px solid #d0d0d0; border-radius: 4px; }"
        "QLabel { background: transparent; color: #000000; }"));

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 10, 14, 10);
    layout->setSpacing(6);

    auto* titleLabel = new QLabel(title, card);
    auto* actualValueLabel = new QLabel(value, card);
    actualValueLabel->setWordWrap(true);
    actualValueLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 700; color: #000000;"));
    if (valueLabel != nullptr) {
        *valueLabel = actualValueLabel;
    }

    layout->addWidget(titleLabel);
    layout->addStretch(1);
    layout->addWidget(actualValueLabel);
    return card;
}

void DiagnosticsPage::setValue(QLabel* label, const QString& value)
{
    if (label != nullptr) {
        label->setText(value);
    }
}

} // namespace upkun::ui
