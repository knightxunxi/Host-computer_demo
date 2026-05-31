#pragma once

#include "domain/Batch.h"
#include "domain/DeviceTypes.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStringList>
#include <QTableWidget>
#include <QVector>
#include <QWidget>

namespace upkun::ui {

class BatchPage final : public QWidget {
    Q_OBJECT

public:
    explicit BatchPage(QWidget* parent = nullptr);

public slots:
    void setCurrentContext(const QString& recipeName, const QString& operatorName, int targetCount);
    void setActiveBatch(const upkun::domain::ProductionBatch& batch, const upkun::domain::DeviceSnapshot& snapshot);
    void clearActiveBatch();
    void setBatchRows(const QVector<QStringList>& rows);
    void setMessage(const QString& message);

signals:
    void startBatchRequested(const QString& batchNo, int targetCount);
    void endBatchRequested();
    void refreshRequested();

private:
    QTableWidget* createTable(const QStringList& headers);
    void fillTable(QTableWidget* table, const QVector<QStringList>& rows);
    QString generatedBatchNo() const;
    QString rateText(int goodCount, int totalCount) const;

    QLineEdit* m_batchNoEdit = nullptr;
    QSpinBox* m_targetCountSpin = nullptr;
    QLabel* m_contextRecipeLabel = nullptr;
    QLabel* m_contextUserLabel = nullptr;
    QLabel* m_activeBatchLabel = nullptr;
    QLabel* m_activeStatusLabel = nullptr;
    QLabel* m_activeRecipeLabel = nullptr;
    QLabel* m_activeTargetLabel = nullptr;
    QLabel* m_activeTotalLabel = nullptr;
    QLabel* m_activeGoodLabel = nullptr;
    QLabel* m_activeBadLabel = nullptr;
    QLabel* m_activeRateLabel = nullptr;
    QLabel* m_messageLabel = nullptr;
    QPushButton* m_startButton = nullptr;
    QPushButton* m_endButton = nullptr;
    QTableWidget* m_batchTable = nullptr;
};

} // namespace upkun::ui
