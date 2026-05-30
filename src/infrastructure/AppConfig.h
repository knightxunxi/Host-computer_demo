#pragma once

#include "domain/DeviceTypes.h"

#include <QString>

namespace upkun::infrastructure {

struct AppConfig {
    QString databasePath = QStringLiteral("data/app.sqlite3");
    QString logPath = QStringLiteral("logs");
    upkun::domain::DeviceConnectionConfig device;

    static AppConfig defaults();
};

} // namespace upkun::infrastructure
