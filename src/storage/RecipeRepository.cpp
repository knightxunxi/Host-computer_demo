#include "storage/RecipeRepository.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {

QString nowIso()
{
    return QDateTime::currentDateTime().toString(Qt::ISODate);
}

void bindRecipe(QSqlQuery* query, const upkun::domain::RecipeParameters& recipe)
{
    query->bindValue(QStringLiteral(":name"), recipe.name);
    query->bindValue(QStringLiteral(":target_speed"), recipe.targetSpeed);
    query->bindValue(QStringLiteral(":fill_volume"), recipe.fillVolumeMl);
    query->bindValue(QStringLiteral(":fill_time"), recipe.fillTimeMs);
    query->bindValue(QStringLiteral(":capping_torque"), recipe.cappingTorqueCentinewtonMeter);
    query->bindValue(QStringLiteral(":weight_min"), recipe.weightMinGram);
    query->bindValue(QStringLiteral(":weight_max"), recipe.weightMaxGram);
    query->bindValue(QStringLiteral(":label_mode"), recipe.labelMode);
    query->bindValue(QStringLiteral(":batch_target_count"), recipe.batchTargetCount);
    query->bindValue(QStringLiteral(":simulation_quality_rate"), recipe.simulationQualityRate);
    query->bindValue(QStringLiteral(":updated_at"), nowIso());
}

upkun::domain::RecipeParameters recipeFromQuery(const QSqlQuery& query)
{
    upkun::domain::RecipeParameters recipe;
    recipe.id = query.value(QStringLiteral("id")).toInt();
    recipe.name = query.value(QStringLiteral("name")).toString();
    recipe.targetSpeed = query.value(QStringLiteral("target_speed")).toInt();
    recipe.fillVolumeMl = query.value(QStringLiteral("fill_volume")).toInt();
    recipe.fillTimeMs = query.value(QStringLiteral("fill_time")).toInt();
    recipe.cappingTorqueCentinewtonMeter = query.value(QStringLiteral("capping_torque")).toInt();
    recipe.weightMinGram = query.value(QStringLiteral("weight_min")).toInt();
    recipe.weightMaxGram = query.value(QStringLiteral("weight_max")).toInt();
    recipe.labelMode = query.value(QStringLiteral("label_mode")).toInt();
    recipe.batchTargetCount = query.value(QStringLiteral("batch_target_count")).toInt();
    recipe.simulationQualityRate = query.value(QStringLiteral("simulation_quality_rate")).toInt();
    return recipe;
}

} // namespace

namespace upkun::storage {

bool RecipeRepository::ensureDefaultRecipe(QString* errorMessage)
{
    QSqlQuery exists;
    exists.prepare(QStringLiteral("SELECT id FROM recipes WHERE name = :name LIMIT 1"));
    exists.bindValue(QStringLiteral(":name"), QStringLiteral("默认配方"));
    if (!exists.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = exists.lastError().text();
        }
        return false;
    }
    if (exists.next()) {
        return true;
    }

    return save(upkun::domain::RecipeParameters {}, errorMessage);
}

bool RecipeRepository::save(const upkun::domain::RecipeParameters& recipe, QString* errorMessage)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO recipes(name, target_speed, fill_volume, fill_time, capping_torque, weight_min, weight_max, "
        "label_mode, batch_target_count, simulation_quality_rate, updated_at) "
        "VALUES(:name, :target_speed, :fill_volume, :fill_time, :capping_torque, :weight_min, :weight_max, "
        ":label_mode, :batch_target_count, :simulation_quality_rate, :updated_at) "
        "ON CONFLICT(name) DO UPDATE SET "
        "target_speed = excluded.target_speed, fill_volume = excluded.fill_volume, fill_time = excluded.fill_time, "
        "capping_torque = excluded.capping_torque, weight_min = excluded.weight_min, weight_max = excluded.weight_max, "
        "label_mode = excluded.label_mode, batch_target_count = excluded.batch_target_count, "
        "simulation_quality_rate = excluded.simulation_quality_rate, updated_at = excluded.updated_at"));
    bindRecipe(&query, recipe);

    if (query.exec()) {
        return true;
    }
    if (errorMessage != nullptr) {
        *errorMessage = query.lastError().text();
    }
    return false;
}

upkun::domain::RecipeParameters RecipeRepository::loadDefault() const
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT * FROM recipes WHERE name = :name LIMIT 1"));
    query.bindValue(QStringLiteral(":name"), QStringLiteral("默认配方"));
    if (query.exec() && query.next()) {
        return recipeFromQuery(query);
    }
    return {};
}

} // namespace upkun::storage
