#pragma once

#include "domain/Recipe.h"

#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QStringList>
#include <QTableWidget>
#include <QVector>
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
    void setRecipes(const QVector<upkun::domain::RecipeParameters>& recipes);
    void setApplyRows(const QVector<QStringList>& rows);
    void setMessage(const QString& message);

signals:
    void saveRequested(upkun::domain::RecipeParameters recipe);
    void applyRequested(upkun::domain::RecipeParameters recipe);
    void copyRequested(upkun::domain::RecipeParameters recipe, QString newName);
    void recipeSelected(int recipeId);
    void refreshRequested();

private:
    QSpinBox* makeSpinBox(int min, int max, int value, const QString& suffix = {});
    void addRow(QGridLayout* layout, int row, const QString& label, QWidget* editor);
    QTableWidget* createTable(const QStringList& headers);
    void fillTable(QTableWidget* table, const QVector<QStringList>& rows);
    QString copiedRecipeName() const;
    QString formatTime(const QDateTime& value) const;

    int m_recipeId = 0;
    int m_recipeVersion = 1;
    QLineEdit* m_nameEdit = nullptr;
    QLabel* m_versionLabel = nullptr;
    QLabel* m_updatedByLabel = nullptr;
    QLabel* m_lastAppliedLabel = nullptr;
    QLabel* m_messageLabel = nullptr;
    QSpinBox* m_targetSpeedSpin = nullptr;
    QSpinBox* m_fillVolumeSpin = nullptr;
    QSpinBox* m_fillTimeSpin = nullptr;
    QSpinBox* m_torqueSpin = nullptr;
    QSpinBox* m_weightMinSpin = nullptr;
    QSpinBox* m_weightMaxSpin = nullptr;
    QSpinBox* m_labelModeSpin = nullptr;
    QSpinBox* m_targetCountSpin = nullptr;
    QSpinBox* m_qualityRateSpin = nullptr;
    QTableWidget* m_recipeTable = nullptr;
    QTableWidget* m_applyTable = nullptr;
};

} // namespace upkun::ui
