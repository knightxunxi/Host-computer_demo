#pragma once

#include <QString>
#include <QStringList>

namespace upkun::storage {

class DatabaseManager final {
public:
    DatabaseManager() = default;

    bool open(const QString& databasePath, QString* errorMessage = nullptr);
    QStringList availableDrivers() const;

private:
    bool initializeSchema(QString* errorMessage);
};

} // namespace upkun::storage
