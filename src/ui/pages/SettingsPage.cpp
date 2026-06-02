#include "ui/pages/SettingsPage.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace upkun::ui {

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(QStringLiteral(
        "QWidget { background: #ffffff; color: #000000; }"
        "QLineEdit, QComboBox, QSpinBox { background: #ffffff; color: #000000; border: 1px solid #a0a0a0; min-height: 28px; padding: 2px 6px; }"
        "QCheckBox { color: #000000; min-height: 28px; }"
        "QPushButton { background: #f5f5f5; color: #000000; border: 1px solid #a0a0a0; min-height: 32px; padding: 0 14px; }"
        "QPushButton:hover { background: #eeeeee; }"
        "QPushButton:pressed { background: #dddddd; }"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    auto* title = new QLabel(QStringLiteral("系统设置"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    auto* hint = new QLabel(QStringLiteral("用于配置通信模式、模拟器启动方式、数据库和日志路径。保存后会写入 config/app.ini。"), this);
    hint->setWordWrap(true);
    rootLayout->addWidget(hint);

    auto* grid = new QGridLayout();
    grid->setColumnStretch(1, 1);
    grid->setHorizontalSpacing(18);
    grid->setVerticalSpacing(12);

    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItem(QStringLiteral("Modbus TCP"), QStringLiteral("modbus_tcp"));
    m_modeCombo->addItem(QStringLiteral("Modbus RTU"), QStringLiteral("modbus_rtu"));
    m_hostEdit = new QLineEdit(this);
    m_portSpin = makeSpinBox(1, 65535, 1502);
    m_serialPortEdit = new QLineEdit(this);
    m_baudRateSpin = makeSpinBox(1200, 921600, 9600);
    m_slaveIdSpin = makeSpinBox(1, 247, 1);
    m_autoStartSimulatorCheck = new QCheckBox(QStringLiteral("上位机启动时自动启动本机模拟器"), this);
    m_statusPollSpin = makeSpinBox(100, 10000, 500);
    m_processPollSpin = makeSpinBox(100, 10000, 1000);
    m_timeoutSpin = makeSpinBox(500, 30000, 2000);
    m_reconnectSpin = makeSpinBox(500, 30000, 3000);
    m_databasePathEdit = new QLineEdit(this);
    m_logPathEdit = new QLineEdit(this);

    addRow(grid, 0, QStringLiteral("通信模式"), m_modeCombo);
    addRow(grid, 1, QStringLiteral("TCP 地址"), m_hostEdit);
    addRow(grid, 2, QStringLiteral("TCP 端口"), m_portSpin);
    addRow(grid, 3, QStringLiteral("串口名称"), m_serialPortEdit);
    addRow(grid, 4, QStringLiteral("波特率"), m_baudRateSpin);
    addRow(grid, 5, QStringLiteral("从站 ID"), m_slaveIdSpin);
    addRow(grid, 6, QStringLiteral("模拟器"), m_autoStartSimulatorCheck);
    addRow(grid, 7, QStringLiteral("状态轮询 ms"), m_statusPollSpin);
    addRow(grid, 8, QStringLiteral("过程轮询 ms"), m_processPollSpin);
    addRow(grid, 9, QStringLiteral("超时 ms"), m_timeoutSpin);
    addRow(grid, 10, QStringLiteral("重连 ms"), m_reconnectSpin);
    addRow(grid, 11, QStringLiteral("数据库路径"), m_databasePathEdit);
    addRow(grid, 12, QStringLiteral("日志目录"), m_logPathEdit);
    rootLayout->addLayout(grid);

    auto* buttons = new QHBoxLayout();
    auto* saveButton = new QPushButton(QStringLiteral("保存配置"), this);
    auto* saveReconnectButton = new QPushButton(QStringLiteral("保存并重连"), this);
    auto* connectButton = new QPushButton(QStringLiteral("连接设备"), this);
    auto* disconnectButton = new QPushButton(QStringLiteral("断开设备"), this);
    buttons->addWidget(saveButton);
    buttons->addWidget(saveReconnectButton);
    buttons->addWidget(connectButton);
    buttons->addWidget(disconnectButton);
    buttons->addStretch(1);
    rootLayout->addLayout(buttons);

    m_messageLabel = new QLabel(QStringLiteral("设置状态：未操作"), this);
    m_messageLabel->setStyleSheet(QStringLiteral("font-weight: 600; color: #000000;"));
    rootLayout->addWidget(m_messageLabel);
    rootLayout->addStretch(1);

    connect(saveButton, &QPushButton::clicked, this, [this] {
        emit configSaveRequested(currentConfig(), false);
    });
    connect(saveReconnectButton, &QPushButton::clicked, this, [this] {
        emit configSaveRequested(currentConfig(), true);
    });
    connect(connectButton, &QPushButton::clicked, this, &SettingsPage::connectRequested);
    connect(disconnectButton, &QPushButton::clicked, this, &SettingsPage::disconnectRequested);
}

void SettingsPage::setConfig(const upkun::infrastructure::AppConfig& config)
{
    const QSignalBlocker modeBlocker(m_modeCombo);
    const int modeIndex = m_modeCombo->findData(config.device.mode);
    m_modeCombo->setCurrentIndex(modeIndex >= 0 ? modeIndex : 0);
    m_hostEdit->setText(config.device.host);
    m_portSpin->setValue(config.device.port);
    m_serialPortEdit->setText(config.device.serialPort);
    m_baudRateSpin->setValue(config.device.baudRate);
    m_slaveIdSpin->setValue(config.device.slaveId);
    m_autoStartSimulatorCheck->setChecked(config.autoStartSimulator);
    m_statusPollSpin->setValue(config.device.statusPollMs);
    m_processPollSpin->setValue(config.device.processPollMs);
    m_timeoutSpin->setValue(config.device.timeoutMs);
    m_reconnectSpin->setValue(config.device.reconnectMs);
    m_databasePathEdit->setText(config.databasePath);
    m_logPathEdit->setText(config.logPath);
}

void SettingsPage::setMessage(const QString& message)
{
    if (m_messageLabel != nullptr) {
        m_messageLabel->setText(QStringLiteral("设置状态：%1").arg(message));
    }
}

upkun::infrastructure::AppConfig SettingsPage::currentConfig() const
{
    upkun::infrastructure::AppConfig config;
    config.device.mode = m_modeCombo->currentData().toString();
    config.device.host = m_hostEdit->text().trimmed().isEmpty() ? QStringLiteral("127.0.0.1") : m_hostEdit->text().trimmed();
    config.device.port = static_cast<quint16>(m_portSpin->value());
    config.device.serialPort = m_serialPortEdit->text().trimmed().isEmpty() ? QStringLiteral("COM1") : m_serialPortEdit->text().trimmed();
    config.device.baudRate = m_baudRateSpin->value();
    config.device.slaveId = m_slaveIdSpin->value();
    config.autoStartSimulator = m_autoStartSimulatorCheck->isChecked();
    config.device.statusPollMs = m_statusPollSpin->value();
    config.device.processPollMs = m_processPollSpin->value();
    config.device.timeoutMs = m_timeoutSpin->value();
    config.device.reconnectMs = m_reconnectSpin->value();
    config.databasePath = m_databasePathEdit->text().trimmed().isEmpty() ? QStringLiteral("data/app.sqlite3") : m_databasePathEdit->text().trimmed();
    config.logPath = m_logPathEdit->text().trimmed().isEmpty() ? QStringLiteral("logs") : m_logPathEdit->text().trimmed();
    return config;
}

QSpinBox* SettingsPage::makeSpinBox(int min, int max, int value)
{
    auto* spinBox = new QSpinBox(this);
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    return spinBox;
}

void SettingsPage::addRow(QGridLayout* layout, int row, const QString& label, QWidget* editor)
{
    auto* labelWidget = new QLabel(label, this);
    layout->addWidget(labelWidget, row, 0);
    layout->addWidget(editor, row, 1);
}

} // namespace upkun::ui
