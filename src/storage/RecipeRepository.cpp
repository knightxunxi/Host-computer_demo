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
}

upkun::domain::RecipeParameters recipeFromQuery(const QSqlQuery& query)
{
    upkun::domain::RecipeParameters recipe;
    recipe.id = query.value(QStringLiteral("id")).toInt();
    recipe.name = query.value(QStringLiteral("name")).toString();
    recipe.version = query.value(QStringLiteral("version")).toInt();
    recipe.targetSpeed = query.value(QStringLiteral("target_speed")).toInt();
    recipe.fillVolumeMl = query.value(QStringLiteral("fill_volume")).toInt();
    recipe.fillTimeMs = query.value(QStringLiteral("fill_time")).toInt();
    recipe.cappingTorqueCentinewtonMeter = query.value(QStringLiteral("capping_torque")).toInt();
    recipe.weightMinGram = query.value(QStringLiteral("weight_min")).toInt();
    recipe.weightMaxGram = query.value(QStringLiteral("weight_max")).toInt();
    recipe.labelMode = query.value(QStringLiteral("label_mode")).toInt();
    recipe.batchTargetCount = query.value(QStringLiteral("batch_target_count")).toInt();
    recipe.simulationQualityRate = query.value(QStringLiteral("simulation_quality_rate")).toInt();
    recipe.updatedBy = query.value(QStringLiteral("updated_by")).toString();

    const QString createdAt = query.value(QStringLiteral("created_at")).toString();
    if (!createdAt.isEmpty()) {
        recipe.createdAt = QDateTime::fromString(createdAt, Qt::ISODate);
    }
    const QString updatedAt = query.value(QStringLiteral("updated_at")).toString();
    if (!updatedAt.isEmpty()) {
        recipe.updatedAt = QDateTime::fromString(updatedAt, Qt::ISODate);
    }
    const QString lastAppliedAt = query.value(QStringLiteral("last_applied_at")).toString();
    if (!lastAppliedAt.isEmpty()) {
        recipe.lastAppliedAt = QDateTime::fromString(lastAppliedAt, Qt::ISODate);
    }
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

    return save(upkun::domain::RecipeParameters {}, QStringLiteral("系统"), errorMessage);
}

bool RecipeRepository::save(const upkun::domain::RecipeParameters& recipe, const QString& updatedBy, QString* errorMessage)
{
    const QString timestamp = nowIso();
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO recipes(name, version, target_speed, fill_volume, fill_time, capping_torque, weight_min, weight_max, "
        "label_mode, batch_target_count, simulation_quality_rate, created_at, updated_at, updated_by) "
        "VALUES(:name, 1, :target_speed, :fill_volume, :fill_time, :capping_torque, :weight_min, :weight_max, "
        ":label_mode, :batch_target_count, :simulation_quality_rate, :created_at, :updated_at, :updated_by) "
        "ON CONFLICT(name) DO UPDATE SET "
        "version = recipes.version + 1, target_speed = excluded.target_speed, fill_volume = excluded.fill_volume, fill_time = excluded.fill_time, "
        "capping_torque = excluded.capping_torque, weight_min = excluded.weight_min, weight_max = excluded.weight_max, "
        "label_mode = excluded.label_mode, batch_target_count = excluded.batch_target_count, "
        "simulation_quality_rate = excluded.simulation_quality_rate, updated_at = excluded.updated_at, updated_by = excluded.updated_by"));
    bindRecipe(&query, recipe);
    query.bindValue(QStringLiteral(":created_at"), timestamp);
    query.bindValue(QStringLiteral(":updated_at"), timestamp);
    query.bindValue(QStringLiteral(":updated_by"), updatedBy.trimmed().isEmpty() ? QStringLiteral("系统") : updatedBy.trimmed());

    if (query.exec()) {
        return true;
    }
    if (errorMessage != nullptr) {
        *errorMessage = query.lastError().text();
    }
    return false;
}

bool RecipeRepository::copyRecipe(int sourceRecipeId, const QString& newName, const QString& updatedBy, upkun::domain::RecipeParameters* copiedRecipe, QString* errorMessage)
{
    const QString normalizedName = newName.trimmed();
    if (normalizedName.isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("新配方名称不能为空");
        }
        return false;
    }
    if (loadByName(normalizedName).has_value()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("配方名称已存在");
        }
        return false;
    }

    const auto source = loadById(sourceRecipeId);
    if (!source.has_value()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("源配方不存在");
        }
        return false;
    }

    auto copied = *source;
    copied.id = 0;
    copied.name = normalizedName;
    copied.version = 1;
    if (!save(copied, updatedBy, errorMessage)) {
        return false;
    }

    const auto saved = loadByName(normalizedName);
    if (!saved.has_value()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("复制后无法读取新配方");
        }
        return false;
    }
    if (copiedRecipe != nullptr) {
        *copiedRecipe = *saved;
    }
    return true;
}

bool RecipeRepository::recordApply(const upkun::domain::RecipeParameters& recipe, const upkun::domain::User& user, const QString& target, const QString& result, const QString& message, QString* errorMessage)
{
    const QString timestamp = nowIso();
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO recipe_apply_logs(recipe_id, recipe_name, recipe_version, user_id, login_name, display_name, role, target, result, message, created_at) "
        "VALUES(:recipe_id, :recipe_name, :recipe_version, :user_id, :login_name, :display_name, :role, :target, :result, :message, :created_at)"));
    query.bindValue(QStringLiteral(":recipe_id"), recipe.id);
    query.bindValue(QStringLiteral(":recipe_name"), recipe.name);
    query.bindValue(QStringLiteral(":recipe_version"), recipe.version);
    query.bindValue(QStringLiteral(":user_id"), user.id);
    query.bindValue(QStringLiteral(":login_name"), user.loginName);
    query.bindValue(QStringLiteral(":display_name"), user.displayName);
    query.bindValue(QStringLiteral(":role"), upkun::domain::userRoleText(user.role));
    query.bindValue(QStringLiteral(":target"), target);
    query.bindValue(QStringLiteral(":result"), result);
    query.bindValue(QStringLiteral(":message"), message);
    query.bindValue(QStringLiteral(":created_at"), timestamp);
    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    QSqlQuery updateQuery;
    updateQuery.prepare(QStringLiteral("UPDATE recipes SET last_applied_at = :last_applied_at WHERE id = :id"));
    updateQuery.bindValue(QStringLiteral(":last_applied_at"), timestamp);
    updateQuery.bindValue(QStringLiteral(":id"), recipe.id);
    if (updateQuery.exec()) {
        return true;
    }
    if (errorMessage != nullptr) {
        *errorMessage = updateQuery.lastError().text();
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

std::optional<upkun::domain::RecipeParameters> RecipeRepository::loadById(int id) const
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT * FROM recipes WHERE id = :id LIMIT 1"));
    query.bindValue(QStringLiteral(":id"), id);
    if (query.exec() && query.next()) {
        return recipeFromQuery(query);
    }
    return std::nullopt;
}

std::optional<upkun::domain::RecipeParameters> RecipeRepository::loadByName(const QString& name) const
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT * FROM recipes WHERE name = :name LIMIT 1"));
    query.bindValue(QStringLiteral(":name"), name.trimmed());
    if (query.exec() && query.next()) {
        return recipeFromQuery(query);
    }
    return std::nullopt;
}

QVector<upkun::domain::RecipeParameters> RecipeRepository::listRecipes() const
{
    QVector<upkun::domain::RecipeParameters> recipes;
    QSqlQuery query(QStringLiteral("SELECT * FROM recipes ORDER BY updated_at DESC, id DESC"));
    while (query.next()) {
        recipes.append(recipeFromQuery(query));
    }
    return recipes;
}

QVector<QStringList> RecipeRepository::recentApplyRows(int limit) const
{
    QVector<QStringList> rows;
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT created_at, recipe_name, recipe_version, display_name, role, target, result, COALESCE(message, '') "
        "FROM recipe_apply_logs ORDER BY id DESC LIMIT :limit"));
    query.bindValue(QStringLiteral(":limit"), limit);
    if (!query.exec()) {
        return rows;
    }

    while (query.next()) {
        QStringList row;
        row.append(query.value(0).toString());
        row.append(query.value(1).toString());
        row.append(QStringLiteral("V%1").arg(query.value(2).toInt()));
        row.append(query.value(3).toString());
        row.append(query.value(4).toString());
        row.append(query.value(5).toString());
        row.append(query.value(6).toString());
        row.append(query.value(7).toString());
        rows.append(row);
    }
    return rows;
}

} // namespace upkun::storage
