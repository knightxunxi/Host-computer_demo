#pragma once

#include "device/IDeviceClient.h"

#include <QByteArray>
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

private slots:
    void poll();
    void handleConnected();
    void handleDisconnected();
    void handleError(QAbstractSocket::SocketError socketError);
    void handleReadyRead();
    void reconnect();

private:
    enum class RequestKind {
        ReadDiscreteInputs,
        ReadInputRegisters,
        WriteCommand,
        WriteRecipe
    };

    struct PendingRequest {
        RequestKind kind = RequestKind::ReadInputRegisters;
        upkun::domain::DeviceCommand command = upkun::domain::DeviceCommand::Start;
    };

    void sendReadRequest(quint8 function, quint16 address, quint16 quantity, RequestKind kind);
    void sendWriteSingleCoil(quint16 address, bool value, upkun::domain::DeviceCommand command);
    void sendWriteMultipleRegisters(quint16 address, const QVector<quint16>& values);
    void sendAdu(quint8 function, const QByteArray& payload, PendingRequest request);
    void processResponse(const QByteArray& adu);
    void decodeDiscreteInputs(const QByteArray& pdu);
    void decodeInputRegisters(const QByteArray& pdu);
    quint16 nextTransactionId();
    void updateConnectionState(upkun::domain::ConnectionState state);

    QTcpSocket m_socket;
    QTimer m_pollTimer;
    QTimer m_reconnectTimer;
    QByteArray m_buffer;
    QHash<quint16, PendingRequest> m_pending;
    upkun::domain::DeviceConnectionConfig m_config;
    upkun::domain::DeviceSnapshot m_snapshot;
    quint16 m_transactionId = 0;
    upkun::domain::ConnectionState m_connectionState = upkun::domain::ConnectionState::Disconnected;
};

} // namespace upkun::device
