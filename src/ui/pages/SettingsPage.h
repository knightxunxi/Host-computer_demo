#pragma once

#include "infrastructure/AppConfig.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>

class QGridLayout;

namespace upkun::ui {

class SettingsPage final : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);

public slots:
    void setConfig(const upkun::infrastructure::AppConfig& config);
    void setMessage(const QString& message);

signals:
    void configSaveRequested(upkun::infrastructure::AppConfig config, bool reconnect);
    void connectRequested();
    void disconnectRequested();

private:
    upkun::infrastructure::AppConfig currentConfig() const;
    QSpinBox* makeSpinBox(int min, int max, int value);
    void addRow(QGridLayout* layout, int row, const QString& label, QWidget* editor);

    QComboBox* m_modeCombo = nullptr;
    QLineEdit* m_hostEdit = nullptr;
    QSpinBox* m_portSpin = nullptr;
    QLineEdit* m_serialPortEdit = nullptr;
    QSpinBox* m_baudRateSpin = nullptr;
    QSpinBox* m_slaveIdSpin = nullptr;
    QCheckBox* m_autoStartSimulatorCheck = nullptr;
    QSpinBox* m_statusPollSpin = nullptr;
    QSpinBox* m_processPollSpin = nullptr;
    QSpinBox* m_timeoutSpin = nullptr;
    QSpinBox* m_reconnectSpin = nullptr;
    QLineEdit* m_databasePathEdit = nullptr;
    QLineEdit* m_logPathEdit = nullptr;
    QLabel* m_messageLabel = nullptr;
};

} // namespace upkun::ui
