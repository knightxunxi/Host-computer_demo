#include "ui/pages/RecipePage.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace upkun::ui {

RecipePage::RecipePage(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(QStringLiteral(
        "QWidget { background: #ffffff; color: #000000; }"
        "QLineEdit, QSpinBox { background: #ffffff; color: #000000; border: 1px solid #a0a0a0; min-height: 28px; padding: 2px 6px; }"
        "QPushButton { background: #f5f5f5; color: #000000; border: 1px solid #a0a0a0; min-height: 32px; padding: 0 14px; }"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    auto* title = new QLabel(QStringLiteral("参数/配方"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    auto* grid = new QGridLayout();
    grid->setColumnStretch(1, 1);

    m_nameEdit = new QLineEdit(QStringLiteral("默认配方"), this);
    m_targetSpeedSpin = makeSpinBox(1, 300, 60, QStringLiteral(" pcs/min"));
    m_fillVolumeSpin = makeSpinBox(1, 5000, 500, QStringLiteral(" ml"));
    m_fillTimeSpin = makeSpinBox(1, 30000, 1000, QStringLiteral(" ms"));
    m_torqueSpin = makeSpinBox(1, 1000, 125, QStringLiteral(" x0.01Nm"));
    m_weightMinSpin = makeSpinBox(1, 10000, 480, QStringLiteral(" g"));
    m_weightMaxSpin = makeSpinBox(1, 10000, 520, QStringLiteral(" g"));
    m_labelModeSpin = makeSpinBox(0, 10, 1);
    m_targetCountSpin = makeSpinBox(1, 1000000, 1000, QStringLiteral(" pcs"));
    m_qualityRateSpin = makeSpinBox(0, 100, 98, QStringLiteral(" %"));

    addRow(grid, 0, QStringLiteral("配方名称"), m_nameEdit);
    addRow(grid, 1, QStringLiteral("目标速度"), m_targetSpeedSpin);
    addRow(grid, 2, QStringLiteral("灌装量"), m_fillVolumeSpin);
    addRow(grid, 3, QStringLiteral("灌装时间"), m_fillTimeSpin);
    addRow(grid, 4, QStringLiteral("旋盖扭矩"), m_torqueSpin);
    addRow(grid, 5, QStringLiteral("重量下限"), m_weightMinSpin);
    addRow(grid, 6, QStringLiteral("重量上限"), m_weightMaxSpin);
    addRow(grid, 7, QStringLiteral("贴标模式"), m_labelModeSpin);
    addRow(grid, 8, QStringLiteral("批次目标"), m_targetCountSpin);
    addRow(grid, 9, QStringLiteral("模拟合格率"), m_qualityRateSpin);

    rootLayout->addLayout(grid);

    auto* buttons = new QHBoxLayout();
    auto* saveButton = new QPushButton(QStringLiteral("保存配方"), this);
    auto* applyButton = new QPushButton(QStringLiteral("下发到PLC/模拟器"), this);
    buttons->addWidget(saveButton);
    buttons->addWidget(applyButton);
    buttons->addStretch(1);
    rootLayout->addLayout(buttons);
    rootLayout->addStretch(1);

    connect(saveButton, &QPushButton::clicked, this, [this] {
        emit saveRequested(currentRecipe());
    });
    connect(applyButton, &QPushButton::clicked, this, [this] {
        emit applyRequested(currentRecipe());
    });
}

upkun::domain::RecipeParameters RecipePage::currentRecipe() const
{
    upkun::domain::RecipeParameters recipe;
    recipe.name = m_nameEdit->text().trimmed().isEmpty() ? QStringLiteral("默认配方") : m_nameEdit->text().trimmed();
    recipe.targetSpeed = m_targetSpeedSpin->value();
    recipe.fillVolumeMl = m_fillVolumeSpin->value();
    recipe.fillTimeMs = m_fillTimeSpin->value();
    recipe.cappingTorqueCentinewtonMeter = m_torqueSpin->value();
    recipe.weightMinGram = m_weightMinSpin->value();
    recipe.weightMaxGram = m_weightMaxSpin->value();
    recipe.labelMode = m_labelModeSpin->value();
    recipe.batchTargetCount = m_targetCountSpin->value();
    recipe.simulationQualityRate = m_qualityRateSpin->value();
    return recipe;
}

void RecipePage::setRecipe(const upkun::domain::RecipeParameters& recipe)
{
    m_nameEdit->setText(recipe.name);
    m_targetSpeedSpin->setValue(recipe.targetSpeed);
    m_fillVolumeSpin->setValue(recipe.fillVolumeMl);
    m_fillTimeSpin->setValue(recipe.fillTimeMs);
    m_torqueSpin->setValue(recipe.cappingTorqueCentinewtonMeter);
    m_weightMinSpin->setValue(recipe.weightMinGram);
    m_weightMaxSpin->setValue(recipe.weightMaxGram);
    m_labelModeSpin->setValue(recipe.labelMode);
    m_targetCountSpin->setValue(recipe.batchTargetCount);
    m_qualityRateSpin->setValue(recipe.simulationQualityRate);
}

QSpinBox* RecipePage::makeSpinBox(int min, int max, int value, const QString& suffix)
{
    auto* spinBox = new QSpinBox(this);
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    spinBox->setSuffix(suffix);
    return spinBox;
}

void RecipePage::addRow(QGridLayout* layout, int row, const QString& label, QWidget* editor)
{
    auto* labelWidget = new QLabel(label, this);
    labelWidget->setStyleSheet(QStringLiteral("color: #000000;"));
    layout->addWidget(labelWidget, row, 0);
    layout->addWidget(editor, row, 1);
}

} // namespace upkun::ui
