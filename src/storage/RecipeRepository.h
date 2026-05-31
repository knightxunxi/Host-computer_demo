#pragma once

#include "domain/Recipe.h"

#include <QString>

namespace upkun::storage {

class RecipeRepository final {
public:
    bool ensureDefaultRecipe(QString* errorMessage = nullptr);
    bool save(const upkun::domain::RecipeParameters& recipe, QString* errorMessage = nullptr);
    upkun::domain::RecipeParameters loadDefault() const;
};

} // namespace upkun::storage
