#pragma once

#include "domain/DeviceTypes.h"

#include <QString>

namespace upkun::infrastructure {

struct AppConfig {
    QString sourcePath;
    QString databasePath = QStringLiteral("data/app.sqlite3");
    QString logPath = QStringLiteral("logs");
    bool autoStartSimulator = true;
    upkun::domain::DeviceConnectionConfig device;

    static AppConfig defaults();
    static AppConfig load(const QString& primaryPath = QStringLiteral("config/app.ini"),
        const QString& fallbackPath = QStringLiteral("config/app.example.ini"));
    static bool save(const AppConfig& config, const QString& path = QStringLiteral("config/app.ini"), QString* errorMessage = nullptr);
};

} // namespace upkun::infrastructure
