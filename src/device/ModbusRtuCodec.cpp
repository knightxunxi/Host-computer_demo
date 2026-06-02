#include "device/ModbusRtuCodec.h"

namespace {

void appendU16BigEndian(QByteArray* data, quint16 value)
{
    data->append(static_cast<char>((value >> 8) & 0xff));
    data->append(static_cast<char>(value & 0xff));
}

QByteArray makeReadRequest(quint8 slaveId, quint8 function, quint16 address, quint16 quantity)
{
    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(function));
    appendU16BigEndian(&frame, address);
    appendU16BigEndian(&frame, quantity);
    return upkun::device::modbus_rtu::appendCrc(frame);
}

} // namespace

namespace upkun::device::modbus_rtu {

quint16 crc16(const QByteArray& data)
{
    quint16 crc = 0xffff;
    for (const char byte : data) {
        crc ^= static_cast<quint8>(byte);
        for (int bit = 0; bit < 8; ++bit) {
            if ((crc & 0x0001) != 0) {
                crc = static_cast<quint16>((crc >> 1) ^ 0xa001);
            } else {
                crc = static_cast<quint16>(crc >> 1);
            }
        }
    }
    return crc;
}

QByteArray appendCrc(const QByteArray& frameWithoutCrc)
{
    QByteArray frame = frameWithoutCrc;
    const quint16 crc = crc16(frameWithoutCrc);
    frame.append(static_cast<char>(crc & 0xff));
    frame.append(static_cast<char>((crc >> 8) & 0xff));
    return frame;
}

bool hasValidCrc(const QByteArray& frame)
{
    if (frame.size() < 4) {
        return false;
    }

    const QByteArray payload = frame.left(frame.size() - 2);
    const auto low = static_cast<quint8>(frame.at(frame.size() - 2));
    const auto high = static_cast<quint8>(frame.at(frame.size() - 1));
    const quint16 expected = static_cast<quint16>((high << 8) | low);
    return crc16(payload) == expected;
}

QByteArray makeReadHoldingRegisters(quint8 slaveId, quint16 address, quint16 quantity)
{
    return makeReadRequest(slaveId, 0x03, address, quantity);
}

QByteArray makeReadInputRegisters(quint8 slaveId, quint16 address, quint16 quantity)
{
    return makeReadRequest(slaveId, 0x04, address, quantity);
}

QByteArray makeWriteSingleCoil(quint8 slaveId, quint16 address, bool value)
{
    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(0x05));
    appendU16BigEndian(&frame, address);
    appendU16BigEndian(&frame, value ? 0xff00 : 0x0000);
    return appendCrc(frame);
}

QByteArray makeWriteSingleRegister(quint8 slaveId, quint16 address, quint16 value)
{
    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(0x06));
    appendU16BigEndian(&frame, address);
    appendU16BigEndian(&frame, value);
    return appendCrc(frame);
}

} // namespace upkun::device::modbus_rtu
