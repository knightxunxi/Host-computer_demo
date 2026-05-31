#include "storage/DatabaseManager.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace upkun::storage {

bool DatabaseManager::open(const QString& databasePath, QString* errorMessage)
{
    const QFileInfo databaseFile(databasePath);
    const QDir databaseDir = databaseFile.absoluteDir();
    if (!databaseDir.exists() && !QDir().mkpath(databaseDir.absolutePath())) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("无法创建数据库目录：%1").arg(databaseDir.absolutePath());
        }
        return false;
    }

    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(databasePath);

    if (!db.open()) {
        if (errorMessage != nullptr) {
            *errorMessage = db.lastError().text();
        }
        return false;
    }

    return initializeSchema(errorMessage);
}

QStringList DatabaseManager::availableDrivers() const
{
    return QSqlDatabase::drivers();
}

bool DatabaseManager::initializeSchema(QString* errorMessage)
{
    QSqlQuery query;

    const QStringList statements {
        QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS alarms (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                alarm_code INTEGER NOT NULL,
                alarm_name TEXT NOT NULL,
                station TEXT NOT NULL,
                level TEXT NOT NULL,
                state TEXT NOT NULL,
                triggered_at TEXT NOT NULL,
                acked_at TEXT,
                acked_by TEXT,
                cleared_at TEXT,
                closed_at TEXT
            )
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE INDEX IF NOT EXISTS idx_alarms_code_state
            ON alarms(alarm_code, state)
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS operation_logs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER,
                login_name TEXT NOT NULL,
                display_name TEXT NOT NULL,
                role TEXT NOT NULL,
                action TEXT NOT NULL,
                target TEXT NOT NULL,
                result TEXT NOT NULL,
                message TEXT,
                created_at TEXT NOT NULL
            )
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE INDEX IF NOT EXISTS idx_operation_logs_created_at
            ON operation_logs(created_at)
        )SQL")
    };

    for (const auto& statement : statements) {
        if (!query.exec(statement)) {
            if (errorMessage != nullptr) {
                *errorMessage = query.lastError().text();
            }
            return false;
        }
    }

    return true;
}

} // namespace upkun::storage
