#pragma once

#include "domain/Recipe.h"
#include "domain/User.h"

#include <QString>
#include <QStringList>
#include <QVector>
#include <optional>

namespace upkun::storage {

class RecipeRepository final {
public:
    bool ensureDefaultRecipe(QString* errorMessage = nullptr);
    bool save(const upkun::domain::RecipeParameters& recipe, const QString& updatedBy, QString* errorMessage = nullptr);
    bool copyRecipe(int sourceRecipeId, const QString& newName, const QString& updatedBy, upkun::domain::RecipeParameters* copiedRecipe, QString* errorMessage = nullptr);
    bool recordApply(const upkun::domain::RecipeParameters& recipe, const upkun::domain::User& user, const QString& target, const QString& result, const QString& message, QString* errorMessage = nullptr);
    upkun::domain::RecipeParameters loadDefault() const;
    std::optional<upkun::domain::RecipeParameters> loadById(int id) const;
    std::optional<upkun::domain::RecipeParameters> loadByName(const QString& name) const;
    QVector<upkun::domain::RecipeParameters> listRecipes() const;
    QVector<QStringList> recentApplyRows(int limit = 20) const;
};

} // namespace upkun::storage
