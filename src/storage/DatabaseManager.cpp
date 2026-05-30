#include "storage/DatabaseManager.h"

#include <QSqlDatabase>
#include <QSqlError>

namespace upkun::storage {

bool DatabaseManager::open(const QString& databasePath, QString* errorMessage)
{
    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(databasePath);

    if (!db.open()) {
        if (errorMessage != nullptr) {
            *errorMessage = db.lastError().text();
        }
        return false;
    }

    return true;
}

QStringList DatabaseManager::availableDrivers() const
{
    return QSqlDatabase::drivers();
}

} // namespace upkun::storage
