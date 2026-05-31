#pragma once

#include <QObject>
#include <QTimer>
#include <QVector>

namespace upkun::simulator {

class LineSimulator final : public QObject {
    Q_OBJECT

public:
    explicit LineSimulator(QObject* parent = nullptr);

    bool readCoil(int offset) const;
    bool readDiscreteInput(int offset) const;
    quint16 readHoldingRegister(int offset) const;
    quint16 readInputRegister(int offset) const;

    bool writeCoil(int offset, bool value);
    bool writeHoldingRegister(int offset, quint16 value);
    bool writeHoldingRegisters(int offset, const QVector<quint16>& values);

public slots:
    void triggerAlarm(int alarmCode);
    void clearAlarm();

signals:
    void stateChanged();

private slots:
    void advanceCycle();

private:
    void resetData();
    void handleCommand(int offset);
    void updateInputs();
    void updateInputRegisters();
    int holdingValue(int offset, int fallback) const;

    QVector<bool> m_coils;
    QVector<bool> m_discreteInputs;
    QVector<quint16> m_holdingRegisters;
    QVector<quint16> m_inputRegisters;
    QTimer m_cycleTimer;
    bool m_running = false;
    int m_activeStation = 0;
    int m_totalCount = 0;
    int m_goodCount = 0;
    int m_badCount = 0;
    int m_batchCount = 0;
    int m_alarmCode = 0;
};

} // namespace upkun::simulator
