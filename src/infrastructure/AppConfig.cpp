#include "infrastructure/AppConfig.h"

#include <QFileInfo>
#include <QSettings>

namespace upkun::infrastructure {

AppConfig AppConfig::defaults()
{
    return {};
}

AppConfig AppConfig::load(const QString& primaryPath, const QString& fallbackPath)
{
    AppConfig config = defaults();
    const QString actualPath = QFileInfo::exists(primaryPath) ? primaryPath : fallbackPath;
    if (!QFileInfo::exists(actualPath)) {
        return config;
    }

    QSettings settings(actualPath, QSettings::IniFormat);
    config.sourcePath = actualPath;
    config.databasePath = settings.value(QStringLiteral("database/path"), config.databasePath).toString();
    config.logPath = settings.value(QStringLiteral("log/path"), config.logPath).toString();
    config.device.mode = settings.value(QStringLiteral("device/mode"), config.device.mode).toString();
    config.device.host = settings.value(QStringLiteral("device/host"), config.device.host).toString();
    config.device.port = static_cast<quint16>(settings.value(QStringLiteral("device/port"), config.device.port).toUInt());
    config.device.serialPort = settings.value(QStringLiteral("device/serial_port"), config.device.serialPort).toString();
    config.device.baudRate = settings.value(QStringLiteral("device/baud_rate"), config.device.baudRate).toInt();
    config.device.slaveId = settings.value(QStringLiteral("device/slave_id"), config.device.slaveId).toInt();
    config.device.statusPollMs = settings.value(QStringLiteral("device/poll_status_ms"), config.device.statusPollMs).toInt();
    config.device.processPollMs = settings.value(QStringLiteral("device/poll_process_ms"), config.device.processPollMs).toInt();
    config.device.timeoutMs = settings.value(QStringLiteral("device/timeout_ms"), config.device.timeoutMs).toInt();
    config.device.reconnectMs = settings.value(QStringLiteral("device/reconnect_ms"), config.device.reconnectMs).toInt();

    if (config.device.port == 0) {
        config.device.port = 1502;
    }
    if (config.device.mode.trimmed().isEmpty()) {
        config.device.mode = QStringLiteral("modbus_tcp");
    }
    config.device.baudRate = qMax(1200, config.device.baudRate);
    config.device.slaveId = qBound(1, config.device.slaveId, 247);
    config.device.statusPollMs = qMax(100, config.device.statusPollMs);
    config.device.processPollMs = qMax(100, config.device.processPollMs);
    config.device.timeoutMs = qMax(500, config.device.timeoutMs);
    config.device.reconnectMs = qMax(500, config.device.reconnectMs);
    return config;
}

} // namespace upkun::infrastructure
