#pragma once

#include <QByteArray>
#include <QtGlobal>

namespace upkun::device::modbus_rtu {

quint16 crc16(const QByteArray& data);
QByteArray appendCrc(const QByteArray& frameWithoutCrc);
bool hasValidCrc(const QByteArray& frame);
QByteArray makeReadHoldingRegisters(quint8 slaveId, quint16 address, quint16 quantity);
QByteArray makeReadInputRegisters(quint8 slaveId, quint16 address, quint16 quantity);
QByteArray makeWriteSingleCoil(quint8 slaveId, quint16 address, bool value);
QByteArray makeWriteSingleRegister(quint8 slaveId, quint16 address, quint16 value);

} // namespace upkun::device::modbus_rtu
