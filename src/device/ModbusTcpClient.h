#pragma once

#include "device/IDeviceClient.h"

#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QTimer>
#include <QTcpSocket>

namespace upkun::device {

class ModbusTcpClient final : public IDeviceClient {
    Q_OBJECT

public:
    explicit ModbusTcpClient(QObject* parent = nullptr);

public slots:
    void connectToDevice(const upkun::domain::DeviceConnectionConfig& config) override;
    void disconnectFromDevice() override;
    void sendCommand(upkun::domain::DeviceCommand command) override;
    void writeRecipe(const upkun::domain::RecipeParameters& recipe) override;
    void injectFault(int alarmCode);

private slots:
    void poll();
    void handleConnected();
    void handleDisconnected();
    void handleError(QAbstractSocket::SocketError socketError);
    void handleReadyRead();
    void reconnect();
    void checkTimeouts();

private:
    enum class RequestKind {
        ReadDiscreteInputs,
        ReadInputRegisters,
        WriteCommand,
        WriteRecipe,
        WriteRegister
    };

    struct PendingRequest {
        RequestKind kind = RequestKind::ReadInputRegisters;
        upkun::domain::DeviceCommand command = upkun::domain::DeviceCommand::Start;
        QDateTime sentAt;
    };

    void sendReadRequest(quint8 function, quint16 address, quint16 quantity, RequestKind kind);
    void sendWriteSingleCoil(quint16 address, bool value, upkun::domain::DeviceCommand command);
    void sendWriteSingleRegister(quint16 address, quint16 value);
    void sendWriteMultipleRegisters(quint16 address, const QVector<quint16>& values);
    void sendAdu(quint8 function, const QByteArray& payload, PendingRequest request);
    void processResponse(const QByteArray& adu);
    void decodeDiscreteInputs(const QByteArray& pdu);
    void decodeInputRegisters(const QByteArray& pdu);
    quint16 nextTransactionId();
    void updateConnectionState(upkun::domain::ConnectionState state);
    void recordError(const QString& message);
    void recordResponse(int roundTripMs);
    void publishDiagnostics();

    QTcpSocket m_socket;
    QTimer m_pollTimer;
    QTimer m_reconnectTimer;
    QTimer m_timeoutTimer;
    QByteArray m_buffer;
    QHash<quint16, PendingRequest> m_pending;
    upkun::domain::DeviceConnectionConfig m_config;
    upkun::domain::DeviceSnapshot m_snapshot;
    upkun::domain::CommunicationDiagnostics m_diagnostics;
    quint16 m_transactionId = 0;
    upkun::domain::ConnectionState m_connectionState = upkun::domain::ConnectionState::Disconnected;
};

} // namespace upkun::device
