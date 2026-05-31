#pragma once

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QStringList>
#include <QTableWidget>
#include <QVector>
#include <QWidget>

namespace upkun::ui {

class AlarmPage final : public QWidget {
    Q_OBJECT

public:
    explicit AlarmPage(QWidget* parent = nullptr);

    QString stateFilter() const;
    QString levelFilter() const;
    QString stationFilter() const;
    QString keywordFilter() const;

public slots:
    void setAlarmRows(const QVector<QStringList>& rows);
    void setOperationRows(const QVector<QStringList>& rows);

signals:
    void refreshRequested();

private:
    QTableWidget* createTable(const QStringList& headers);
    void fillTable(QTableWidget* table, const QVector<QStringList>& rows);
    void updateAlarmDetail(int row);
    void clearAlarmDetail();

    QComboBox* m_stateFilter = nullptr;
    QComboBox* m_levelFilter = nullptr;
    QComboBox* m_stationFilter = nullptr;
    QLineEdit* m_keywordEdit = nullptr;
    QLabel* m_detailTitleLabel = nullptr;
    QLabel* m_detailStateLabel = nullptr;
    QLabel* m_detailAckLabel = nullptr;
    QLabel* m_detailTimeLabel = nullptr;
    QLabel* m_detailSuggestionLabel = nullptr;
    QTableWidget* m_alarmTable = nullptr;
    QTableWidget* m_operationTable = nullptr;
};

} // namespace upkun::ui
