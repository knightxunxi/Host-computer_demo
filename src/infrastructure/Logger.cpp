#include "infrastructure/Logger.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

namespace {

QFile* g_logFile = nullptr;
QMutex g_logMutex;
QtMessageHandler g_previousHandler = nullptr;

QString levelText(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("DEBUG");
    case QtInfoMsg:
        return QStringLiteral("INFO");
    case QtWarningMsg:
        return QStringLiteral("WARN");
    case QtCriticalMsg:
        return QStringLiteral("ERROR");
    case QtFatalMsg:
        return QStringLiteral("FATAL");
    }
    return QStringLiteral("LOG");
}

void fileMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    {
        QMutexLocker locker(&g_logMutex);
        if (g_logFile != nullptr && g_logFile->isOpen()) {
            QTextStream stream(g_logFile);
            stream << QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"))
                   << " [" << levelText(type) << "] " << message << Qt::endl;
        }
    }

    if (g_previousHandler != nullptr) {
        g_previousHandler(type, context, message);
    }
}

} // namespace

namespace upkun::infrastructure {

void initializeLogger(const QString& logDirectory)
{
    QMutexLocker locker(&g_logMutex);
    QDir().mkpath(logDirectory);

    if (g_logFile != nullptr) {
        g_logFile->close();
        delete g_logFile;
        g_logFile = nullptr;
    }

    const QString fileName = QStringLiteral("app-%1.log").arg(QDate::currentDate().toString(QStringLiteral("yyyyMMdd")));
    g_logFile = new QFile(QDir(logDirectory).filePath(fileName));
    g_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

    if (g_previousHandler == nullptr) {
        g_previousHandler = qInstallMessageHandler(fileMessageHandler);
    }
}

void shutdownLogger()
{
    QMutexLocker locker(&g_logMutex);
    if (g_previousHandler != nullptr) {
        qInstallMessageHandler(g_previousHandler);
        g_previousHandler = nullptr;
    }
    if (g_logFile != nullptr) {
        g_logFile->close();
        delete g_logFile;
        g_logFile = nullptr;
    }
}

} // namespace upkun::infrastructure
