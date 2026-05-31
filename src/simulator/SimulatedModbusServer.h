#pragma once

#include "simulator/LineSimulator.h"

#include <QByteArray>
#include <QHash>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>

namespace upkun::simulator {

class SimulatedModbusServer final : public QObject {
    Q_OBJECT

public:
    explicit SimulatedModbusServer(QObject* parent = nullptr);

    bool start(const QHostAddress& address, quint16 port, QString* errorMessage = nullptr);
    void stop();
    bool isListening() const;
    quint16 serverPort() const;

public slots:
    void triggerAlarm(int alarmCode);
    void clearAlarm();

signals:
    void listeningChanged(bool listening);

private slots:
    void handleNewConnection();
    void handleReadyRead();
    void handleDisconnected();

private:
    QByteArray processRequest(const QByteArray& adu);
    QByteArray makeResponse(const QByteArray& adu, const QByteArray& pdu) const;
    QByteArray makeExceptionResponse(const QByteArray& adu, quint8 function, quint8 exceptionCode) const;
    QByteArray readBits(quint8 function, quint16 startAddress, quint16 quantity) const;
    QByteArray readRegisters(quint8 function, quint16 startAddress, quint16 quantity) const;
    bool writeSingleCoil(quint16 address, quint16 value);
    bool writeSingleRegister(quint16 address, quint16 value);
    bool writeMultipleRegisters(quint16 address, const QVector<quint16>& values);

    QTcpServer m_server;
    QHash<QTcpSocket*, QByteArray> m_buffers;
    LineSimulator m_lineSimulator;
};

} // namespace upkun::simulator
