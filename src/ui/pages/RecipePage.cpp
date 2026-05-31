#include "ui/pages/RecipePage.h"

#include <QDateTime>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace upkun::ui {

RecipePage::RecipePage(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(QStringLiteral(
        "QWidget { background: #ffffff; color: #000000; }"
        "QLineEdit, QSpinBox { background: #ffffff; color: #000000; border: 1px solid #a0a0a0; min-height: 28px; padding: 2px 6px; }"
        "QPushButton { background: #f5f5f5; color: #000000; border: 1px solid #a0a0a0; min-height: 32px; padding: 0 14px; }"
        "QPushButton:hover { background: #eeeeee; }"
        "QTableWidget { background: #ffffff; color: #000000; gridline-color: #d0d0d0; }"
        "QHeaderView::section { background: #f5f5f5; color: #000000; border: 1px solid #d0d0d0; padding: 4px; }"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    auto* title = new QLabel(QStringLiteral("参数/配方"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700; color: #000000;"));
    rootLayout->addWidget(title);

    auto* topLayout = new QHBoxLayout();
    topLayout->setSpacing(18);

    auto* editorLayout = new QVBoxLayout();
    auto* grid = new QGridLayout();
    grid->setColumnStretch(1, 1);

    m_nameEdit = new QLineEdit(QStringLiteral("默认配方"), this);
    m_versionLabel = new QLabel(QStringLiteral("版本：V1"), this);
    m_updatedByLabel = new QLabel(QStringLiteral("更新人：系统"), this);
    m_lastAppliedLabel = new QLabel(QStringLiteral("最后下发：未下发"), this);
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
    addRow(grid, 1, QStringLiteral("当前版本"), m_versionLabel);
    addRow(grid, 2, QStringLiteral("更新人"), m_updatedByLabel);
    addRow(grid, 3, QStringLiteral("最后下发"), m_lastAppliedLabel);
    addRow(grid, 4, QStringLiteral("目标速度"), m_targetSpeedSpin);
    addRow(grid, 5, QStringLiteral("灌装量"), m_fillVolumeSpin);
    addRow(grid, 6, QStringLiteral("灌装时间"), m_fillTimeSpin);
    addRow(grid, 7, QStringLiteral("旋盖扭矩"), m_torqueSpin);
    addRow(grid, 8, QStringLiteral("重量下限"), m_weightMinSpin);
    addRow(grid, 9, QStringLiteral("重量上限"), m_weightMaxSpin);
    addRow(grid, 10, QStringLiteral("贴标模式"), m_labelModeSpin);
    addRow(grid, 11, QStringLiteral("批次目标"), m_targetCountSpin);
    addRow(grid, 12, QStringLiteral("模拟合格率"), m_qualityRateSpin);

    editorLayout->addLayout(grid);

    auto* buttons = new QHBoxLayout();
    auto* newButton = new QPushButton(QStringLiteral("新建配方"), this);
    auto* copyButton = new QPushButton(QStringLiteral("复制为新配方"), this);
    auto* saveButton = new QPushButton(QStringLiteral("保存配方"), this);
    auto* applyButton = new QPushButton(QStringLiteral("下发到PLC/模拟器"), this);
    auto* refreshButton = new QPushButton(QStringLiteral("刷新列表"), this);
    buttons->addWidget(newButton);
    buttons->addWidget(copyButton);
    buttons->addWidget(saveButton);
    buttons->addWidget(applyButton);
    buttons->addWidget(refreshButton);
    buttons->addStretch(1);
    editorLayout->addLayout(buttons);

    m_messageLabel = new QLabel(QStringLiteral("配方状态：未操作"), this);
    m_messageLabel->setStyleSheet(QStringLiteral("font-weight: 600; color: #000000;"));
    editorLayout->addWidget(m_messageLabel);
    editorLayout->addStretch(1);

    auto* tableLayout = new QVBoxLayout();
    auto* recipeTableTitle = new QLabel(QStringLiteral("配方列表"), this);
    recipeTableTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700; color: #000000;"));
    tableLayout->addWidget(recipeTableTitle);
    m_recipeTable = createTable({
        QStringLiteral("ID"),
        QStringLiteral("名称"),
        QStringLiteral("版本"),
        QStringLiteral("速度"),
        QStringLiteral("灌装量"),
        QStringLiteral("批次目标"),
        QStringLiteral("更新人"),
        QStringLiteral("更新时间"),
        QStringLiteral("最后下发")
    });
    tableLayout->addWidget(m_recipeTable, 2);

    auto* applyTableTitle = new QLabel(QStringLiteral("最近下发记录"), this);
    applyTableTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700; color: #000000;"));
    tableLayout->addWidget(applyTableTitle);
    m_applyTable = createTable({
        QStringLiteral("时间"),
        QStringLiteral("配方"),
        QStringLiteral("版本"),
        QStringLiteral("用户"),
        QStringLiteral("角色"),
        QStringLiteral("目标"),
        QStringLiteral("结果"),
        QStringLiteral("消息")
    });
    tableLayout->addWidget(m_applyTable, 1);

    topLayout->addLayout(editorLayout, 2);
    topLayout->addLayout(tableLayout, 3);
    rootLayout->addLayout(topLayout, 1);

    connect(newButton, &QPushButton::clicked, this, [this] {
        upkun::domain::RecipeParameters recipe;
        recipe.name = QStringLiteral("新配方%1").arg(QDateTime::currentDateTime().toString(QStringLiteral("HHmmss")));
        setRecipe(recipe);
        setMessage(QStringLiteral("已创建未保存的新配方"));
    });
    connect(copyButton, &QPushButton::clicked, this, [this] {
        emit copyRequested(currentRecipe(), copiedRecipeName());
    });
    connect(saveButton, &QPushButton::clicked, this, [this] {
        emit saveRequested(currentRecipe());
    });
    connect(applyButton, &QPushButton::clicked, this, [this] {
        emit applyRequested(currentRecipe());
    });
    connect(refreshButton, &QPushButton::clicked, this, &RecipePage::refreshRequested);
    connect(m_recipeTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        const auto* item = m_recipeTable->item(row, 0);
        if (item != nullptr) {
            emit recipeSelected(item->text().toInt());
        }
    });
}

upkun::domain::RecipeParameters RecipePage::currentRecipe() const
{
    upkun::domain::RecipeParameters recipe;
    recipe.id = m_recipeId;
    recipe.version = m_recipeVersion;
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
    m_recipeId = recipe.id;
    m_recipeVersion = qMax(1, recipe.version);
    m_nameEdit->setText(recipe.name);
    m_versionLabel->setText(QStringLiteral("版本：V%1").arg(m_recipeVersion));
    m_updatedByLabel->setText(QStringLiteral("更新人：%1").arg(recipe.updatedBy.isEmpty() ? QStringLiteral("系统") : recipe.updatedBy));
    m_lastAppliedLabel->setText(QStringLiteral("最后下发：%1").arg(formatTime(recipe.lastAppliedAt)));
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

void RecipePage::setRecipes(const QVector<upkun::domain::RecipeParameters>& recipes)
{
    if (m_recipeTable == nullptr) {
        return;
    }

    QSignalBlocker blocker(m_recipeTable);
    m_recipeTable->setRowCount(recipes.size());
    for (qsizetype row = 0; row < recipes.size(); ++row) {
        const auto& recipe = recipes.at(row);
        const QStringList values {
            QString::number(recipe.id),
            recipe.name,
            QStringLiteral("V%1").arg(recipe.version),
            QStringLiteral("%1 pcs/min").arg(recipe.targetSpeed),
            QStringLiteral("%1 ml").arg(recipe.fillVolumeMl),
            QStringLiteral("%1 pcs").arg(recipe.batchTargetCount),
            recipe.updatedBy,
            formatTime(recipe.updatedAt),
            formatTime(recipe.lastAppliedAt)
        };
        for (int column = 0; column < values.size(); ++column) {
            auto* item = new QTableWidgetItem(values.at(column));
            item->setForeground(Qt::black);
            m_recipeTable->setItem(static_cast<int>(row), column, item);
        }
    }
}

void RecipePage::setApplyRows(const QVector<QStringList>& rows)
{
    fillTable(m_applyTable, rows);
}

void RecipePage::setMessage(const QString& message)
{
    m_messageLabel->setText(QStringLiteral("配方状态：%1").arg(message));
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

QTableWidget* RecipePage::createTable(const QStringList& headers)
{
    auto* table = new QTableWidget(this);
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    return table;
}

void RecipePage::fillTable(QTableWidget* table, const QVector<QStringList>& rows)
{
    if (table == nullptr) {
        return;
    }

    table->setRowCount(rows.size());
    for (qsizetype row = 0; row < rows.size(); ++row) {
        const auto& values = rows.at(row);
        for (qsizetype column = 0; column < values.size() && column < table->columnCount(); ++column) {
            auto* item = new QTableWidgetItem(values.at(column));
            item->setForeground(Qt::black);
            table->setItem(row, column, item);
        }
    }
}

QString RecipePage::copiedRecipeName() const
{
    return QStringLiteral("%1-副本-%2")
        .arg(currentRecipe().name, QDateTime::currentDateTime().toString(QStringLiteral("HHmmss")));
}

QString RecipePage::formatTime(const QDateTime& value) const
{
    return value.isValid() ? value.toString(Qt::ISODate) : QStringLiteral("未记录");
}

} // namespace upkun::ui
