#pragma once

#include <QDebug>
#include <QString>

namespace upkun::infrastructure {

void initializeLogger(const QString& logDirectory);
void shutdownLogger();

inline void logInfo(const QString& message)
{
    qInfo().noquote() << message;
}

inline void logWarning(const QString& message)
{
    qWarning().noquote() << message;
}

} // namespace upkun::infrastructure
