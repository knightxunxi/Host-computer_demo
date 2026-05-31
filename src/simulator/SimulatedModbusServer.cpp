#include "simulator/SimulatedModbusServer.h"

#include <QDataStream>

namespace {

quint16 readU16(const QByteArray& data, int offset)
{
    const auto high = static_cast<quint8>(data.at(offset));
    const auto low = static_cast<quint8>(data.at(offset + 1));
    return static_cast<quint16>((high << 8) | low);
}

void appendU16(QByteArray* data, quint16 value)
{
    data->append(static_cast<char>((value >> 8) & 0xff));
    data->append(static_cast<char>(value & 0xff));
}

} // namespace

namespace upkun::simulator {

SimulatedModbusServer::SimulatedModbusServer(QObject* parent)
    : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection, this, &SimulatedModbusServer::handleNewConnection);
}

bool SimulatedModbusServer::start(const QHostAddress& address, quint16 port, QString* errorMessage)
{
    if (m_server.isListening()) {
        return true;
    }

    if (!m_server.listen(address, port)) {
        if (errorMessage != nullptr) {
            *errorMessage = m_server.errorString();
        }
        emit listeningChanged(false);
        return false;
    }

    emit listeningChanged(true);
    return true;
}

void SimulatedModbusServer::stop()
{
    const auto sockets = m_buffers.keys();
    for (auto* socket : sockets) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
    m_buffers.clear();
    m_server.close();
    emit listeningChanged(false);
}

bool SimulatedModbusServer::isListening() const
{
    return m_server.isListening();
}

quint16 SimulatedModbusServer::serverPort() const
{
    return m_server.serverPort();
}

void SimulatedModbusServer::triggerAlarm(int alarmCode)
{
    m_lineSimulator.triggerAlarm(alarmCode);
}

void SimulatedModbusServer::clearAlarm()
{
    m_lineSimulator.clearAlarm();
}

void SimulatedModbusServer::handleNewConnection()
{
    while (auto* socket = m_server.nextPendingConnection()) {
        m_buffers.insert(socket, {});
        connect(socket, &QTcpSocket::readyRead, this, &SimulatedModbusServer::handleReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &SimulatedModbusServer::handleDisconnected);
    }
}

void SimulatedModbusServer::handleReadyRead()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket == nullptr) {
        return;
    }

    auto& buffer = m_buffers[socket];
    buffer.append(socket->readAll());

    while (buffer.size() >= 7) {
        const quint16 length = readU16(buffer, 4);
        const int aduSize = 6 + length;
        if (buffer.size() < aduSize) {
            break;
        }

        const QByteArray request = buffer.left(aduSize);
        buffer.remove(0, aduSize);
        socket->write(processRequest(request));
    }
}

void SimulatedModbusServer::handleDisconnected()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket == nullptr) {
        return;
    }

    m_buffers.remove(socket);
    socket->deleteLater();
}

QByteArray SimulatedModbusServer::processRequest(const QByteArray& adu)
{
    if (adu.size() < 8) {
        return {};
    }

    const quint8 function = static_cast<quint8>(adu.at(7));
    const quint16 startAddress = adu.size() >= 10 ? readU16(adu, 8) : 0;

    switch (function) {
    case 0x01:
    case 0x02:
        if (adu.size() < 12) {
            return makeExceptionResponse(adu, function, 0x03);
        }
        return makeResponse(adu, readBits(function, startAddress, readU16(adu, 10)));
    case 0x03:
    case 0x04:
        if (adu.size() < 12) {
            return makeExceptionResponse(adu, function, 0x03);
        }
        return makeResponse(adu, readRegisters(function, startAddress, readU16(adu, 10)));
    case 0x05:
        if (adu.size() < 12 || !writeSingleCoil(startAddress, readU16(adu, 10))) {
            return makeExceptionResponse(adu, function, 0x02);
        }
        return makeResponse(adu, adu.mid(7, 5));
    case 0x06:
        if (adu.size() < 12 || !writeSingleRegister(startAddress, readU16(adu, 10))) {
            return makeExceptionResponse(adu, function, 0x02);
        }
        return makeResponse(adu, adu.mid(7, 5));
    case 0x10: {
        if (adu.size() < 13) {
            return makeExceptionResponse(adu, function, 0x03);
        }
        const quint16 quantity = readU16(adu, 10);
        const quint8 byteCount = static_cast<quint8>(adu.at(12));
        if (byteCount != quantity * 2 || adu.size() < 13 + byteCount) {
            return makeExceptionResponse(adu, function, 0x03);
        }
        QVector<quint16> values;
        values.reserve(quantity);
        for (quint16 i = 0; i < quantity; ++i) {
            values.append(readU16(adu, 13 + i * 2));
        }
        if (!writeMultipleRegisters(startAddress, values)) {
            return makeExceptionResponse(adu, function, 0x02);
        }
        QByteArray pdu;
        pdu.append(static_cast<char>(function));
        appendU16(&pdu, startAddress);
        appendU16(&pdu, quantity);
        return makeResponse(adu, pdu);
    }
    default:
        return makeExceptionResponse(adu, function, 0x01);
    }
}

QByteArray SimulatedModbusServer::makeResponse(const QByteArray& adu, const QByteArray& pdu) const
{
    QByteArray response;
    response.append(adu.left(4));
    appendU16(&response, static_cast<quint16>(pdu.size() + 1));
    response.append(adu.at(6));
    response.append(pdu);
    return response;
}

QByteArray SimulatedModbusServer::makeExceptionResponse(const QByteArray& adu, quint8 function, quint8 exceptionCode) const
{
    QByteArray pdu;
    pdu.append(static_cast<char>(function | 0x80));
    pdu.append(static_cast<char>(exceptionCode));
    return makeResponse(adu, pdu);
}

QByteArray SimulatedModbusServer::readBits(quint8 function, quint16 startAddress, quint16 quantity) const
{
    QByteArray pdu;
    pdu.append(static_cast<char>(function));
    const int byteCount = (quantity + 7) / 8;
    pdu.append(static_cast<char>(byteCount));

    for (int byteIndex = 0; byteIndex < byteCount; ++byteIndex) {
        quint8 packed = 0;
        for (int bitIndex = 0; bitIndex < 8; ++bitIndex) {
            const int pointOffset = startAddress + byteIndex * 8 + bitIndex;
            const bool value = function == 0x01
                ? m_lineSimulator.readCoil(pointOffset)
                : m_lineSimulator.readDiscreteInput(pointOffset);
            if (value) {
                packed |= static_cast<quint8>(1 << bitIndex);
            }
        }
        pdu.append(static_cast<char>(packed));
    }

    return pdu;
}

QByteArray SimulatedModbusServer::readRegisters(quint8 function, quint16 startAddress, quint16 quantity) const
{
    QByteArray pdu;
    pdu.append(static_cast<char>(function));
    pdu.append(static_cast<char>(quantity * 2));

    for (quint16 i = 0; i < quantity; ++i) {
        const quint16 value = function == 0x03
            ? m_lineSimulator.readHoldingRegister(startAddress + i)
            : m_lineSimulator.readInputRegister(startAddress + i);
        appendU16(&pdu, value);
    }

    return pdu;
}

bool SimulatedModbusServer::writeSingleCoil(quint16 address, quint16 value)
{
    return m_lineSimulator.writeCoil(address, value == 0xff00);
}

bool SimulatedModbusServer::writeSingleRegister(quint16 address, quint16 value)
{
    return m_lineSimulator.writeHoldingRegister(address, value);
}

bool SimulatedModbusServer::writeMultipleRegisters(quint16 address, const QVector<quint16>& values)
{
    return m_lineSimulator.writeHoldingRegisters(address, values);
}

} // namespace upkun::simulator
