#pragma once

#include "domain/DeviceTypes.h"

#include <QLabel>
#include <QWidget>

namespace upkun::ui {

class DiagnosticsPage final : public QWidget {
    Q_OBJECT

public:
    explicit DiagnosticsPage(QWidget* parent = nullptr);

public slots:
    void updateDiagnostics(const upkun::domain::CommunicationDiagnostics& diagnostics);

private:
    QWidget* createMetricCard(const QString& title, const QString& value, QLabel** valueLabel);
    void setValue(QLabel* label, const QString& value);

    QLabel* m_stateValueLabel = nullptr;
    QLabel* m_endpointValueLabel = nullptr;
    QLabel* m_qualityValueLabel = nullptr;
    QLabel* m_pendingValueLabel = nullptr;
    QLabel* m_requestValueLabel = nullptr;
    QLabel* m_responseValueLabel = nullptr;
    QLabel* m_timeoutValueLabel = nullptr;
    QLabel* m_errorValueLabel = nullptr;
    QLabel* m_reconnectValueLabel = nullptr;
    QLabel* m_consecutiveTimeoutValueLabel = nullptr;
    QLabel* m_roundTripValueLabel = nullptr;
    QLabel* m_lastRequestValueLabel = nullptr;
    QLabel* m_lastResponseValueLabel = nullptr;
    QLabel* m_lastErrorValueLabel = nullptr;
};

} // namespace upkun::ui
