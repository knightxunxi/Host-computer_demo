#include "storage/TrendRepository.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>
#include <QVariant>

namespace {

QString csvEscape(QString value)
{
    value.replace(QStringLiteral("\""), QStringLiteral("\"\""));
    return QStringLiteral("\"%1\"").arg(value);
}

bool ensureParentDir(const QString& filePath, QString* errorMessage)
{
    const QFileInfo fileInfo(filePath);
    const QDir dir = fileInfo.absoluteDir();
    if (dir.exists() || QDir().mkpath(dir.absolutePath())) {
        return true;
    }
    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("无法创建目录：%1").arg(dir.absolutePath());
    }
    return false;
}

} // namespace

namespace upkun::storage {

bool TrendRepository::appendSample(const upkun::domain::DeviceSnapshot& snapshot, QString* errorMessage)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO trend_samples(sample_time, speed, fill_volume, weight, torque, temperature, pressure) "
        "VALUES(:sample_time, :speed, :fill_volume, :weight, :torque, :temperature, :pressure)"));
    query.bindValue(QStringLiteral(":sample_time"), QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":speed"), snapshot.counters.speed);
    query.bindValue(QStringLiteral(":fill_volume"), snapshot.processValues.fillVolumeMl);
    query.bindValue(QStringLiteral(":weight"), snapshot.processValues.weightGram);
    query.bindValue(QStringLiteral(":torque"), snapshot.processValues.torqueCentinewtonMeter);
    query.bindValue(QStringLiteral(":temperature"), snapshot.processValues.temperatureDeciCelsius);
    query.bindValue(QStringLiteral(":pressure"), snapshot.processValues.pressureCentiMpa);

    if (query.exec()) {
        return true;
    }
    if (errorMessage != nullptr) {
        *errorMessage = query.lastError().text();
    }
    return false;
}

bool TrendRepository::exportCsv(const QString& filePath, QString* errorMessage) const
{
    if (!ensureParentDir(filePath, errorMessage)) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage != nullptr) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << "sample_time,speed,fill_volume,weight,torque,temperature,pressure\n";

    QSqlQuery query(QStringLiteral(
        "SELECT sample_time, speed, fill_volume, weight, torque, temperature, pressure "
        "FROM trend_samples ORDER BY id ASC"));
    while (query.next()) {
        QStringList values;
        for (int i = 0; i < 7; ++i) {
            values.append(csvEscape(query.value(i).toString()));
        }
        stream << values.join(QLatin1Char(',')) << '\n';
    }

    return true;
}

} // namespace upkun::storage
