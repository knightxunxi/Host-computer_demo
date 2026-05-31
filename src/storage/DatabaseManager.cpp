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

    // 学习项目先采用轻量迁移方式：启动时逐条 CREATE IF NOT EXISTS。
    // 真实项目进入多版本交付后，应改成带版本号的数据库迁移脚本。
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
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                login_name TEXT NOT NULL UNIQUE,
                display_name TEXT NOT NULL,
                role TEXT NOT NULL,
                enabled INTEGER NOT NULL DEFAULT 1,
                password_hash TEXT NOT NULL,
                last_login_at TEXT,
                created_at TEXT NOT NULL
            )
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE UNIQUE INDEX IF NOT EXISTS idx_users_login_name
            ON users(login_name)
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
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS recipes (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL UNIQUE,
                target_speed INTEGER NOT NULL,
                fill_volume INTEGER NOT NULL,
                fill_time INTEGER NOT NULL,
                capping_torque INTEGER NOT NULL,
                weight_min INTEGER NOT NULL,
                weight_max INTEGER NOT NULL,
                label_mode INTEGER NOT NULL,
                batch_target_count INTEGER NOT NULL,
                simulation_quality_rate INTEGER NOT NULL,
                updated_at TEXT NOT NULL
            )
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS trend_samples (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                sample_time TEXT NOT NULL,
                speed INTEGER NOT NULL,
                fill_volume INTEGER NOT NULL,
                weight INTEGER NOT NULL,
                torque INTEGER NOT NULL,
                temperature INTEGER NOT NULL,
                pressure INTEGER NOT NULL
            )
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE INDEX IF NOT EXISTS idx_trend_samples_time
            ON trend_samples(sample_time)
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
