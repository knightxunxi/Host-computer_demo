#pragma once

#include "domain/Recipe.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>

class QGridLayout;

namespace upkun::ui {

class RecipePage final : public QWidget {
    Q_OBJECT

public:
    explicit RecipePage(QWidget* parent = nullptr);

    upkun::domain::RecipeParameters currentRecipe() const;

public slots:
    void setRecipe(const upkun::domain::RecipeParameters& recipe);

signals:
    void saveRequested(upkun::domain::RecipeParameters recipe);
    void applyRequested(upkun::domain::RecipeParameters recipe);

private:
    QSpinBox* makeSpinBox(int min, int max, int value, const QString& suffix = {});
    void addRow(QGridLayout* layout, int row, const QString& label, QWidget* editor);

    QLineEdit* m_nameEdit = nullptr;
    QSpinBox* m_targetSpeedSpin = nullptr;
    QSpinBox* m_fillVolumeSpin = nullptr;
    QSpinBox* m_fillTimeSpin = nullptr;
    QSpinBox* m_torqueSpin = nullptr;
    QSpinBox* m_weightMinSpin = nullptr;
    QSpinBox* m_weightMaxSpin = nullptr;
    QSpinBox* m_labelModeSpin = nullptr;
    QSpinBox* m_targetCountSpin = nullptr;
    QSpinBox* m_qualityRateSpin = nullptr;
};

} // namespace upkun::ui
