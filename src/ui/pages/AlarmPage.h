#pragma once

#include <QStringList>
#include <QTableWidget>
#include <QVector>
#include <QWidget>

namespace upkun::ui {

class AlarmPage final : public QWidget {
    Q_OBJECT

public:
    explicit AlarmPage(QWidget* parent = nullptr);

public slots:
    void setAlarmRows(const QVector<QStringList>& rows);
    void setOperationRows(const QVector<QStringList>& rows);

signals:
    void refreshRequested();

private:
    QTableWidget* createTable(const QStringList& headers);
    void fillTable(QTableWidget* table, const QVector<QStringList>& rows);

    QTableWidget* m_alarmTable = nullptr;
    QTableWidget* m_operationTable = nullptr;
};

} // namespace upkun::ui
