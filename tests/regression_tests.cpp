#include "device/ModbusPointMap.h"
#include "simulator/SimulatedModbusServer.h"
#include "storage/DatabaseManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QHostAddress>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QTextStream>
#include <QThread>

namespace {

bool expect(bool condition, const QString& message)
{
    if (!condition) {
        QTextStream(stderr) << "FAIL: " << message << Qt::endl;
        return false;
    }
    return true;
}

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

QByteArray modbusRequest(quint16 transactionId, quint8 function, quint16 address, quint16 value)
{
    QByteArray request;
    appendU16(&request, transactionId);
    appendU16(&request, 0);
    appendU16(&request, 6);
    request.append(static_cast<char>(1));
    request.append(static_cast<char>(function));
    appendU16(&request, address);
    appendU16(&request, value);
    return request;
}

QByteArray readResponse(QTcpSocket* socket)
{
    QByteArray response;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 2000) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        response.append(socket->readAll());
        if (response.size() >= 7) {
            const int aduSize = 6 + readU16(response, 4);
            if (response.size() >= aduSize) {
                return response.left(aduSize);
            }
        }
        QThread::msleep(10);
    }
    return response;
}

bool sendAndExpectFunction(QTcpSocket* socket, const QByteArray& request, quint8 function, QByteArray* response)
{
    socket->write(request);
    socket->flush();
    QElapsedTimer writeTimer;
    writeTimer.start();
    while (socket->bytesToWrite() > 0 && writeTimer.elapsed() < 1000) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        socket->flush();
        QThread::msleep(5);
    }
    if (socket->bytesToWrite() > 0) {
        return expect(false, QStringLiteral("Modbus 请求写入超时"));
    }

    const QByteArray actual = readResponse(socket);
    if (response != nullptr) {
        *response = actual;
    }
    return expect(actual.size() >= 8, QStringLiteral("Modbus 响应长度不足"))
        && expect(static_cast<quint8>(actual.at(7)) == function, QStringLiteral("Modbus 响应功能码不匹配"));
}

bool testPointMap()
{
    using namespace upkun::device::modbus;
    return expect(toCoilOffset(Coils::CmdStart) == 0, QStringLiteral("启动线圈偏移应为 0"))
        && expect(toCoilOffset(Coils::CmdSimFault) == 9, QStringLiteral("模拟故障线圈偏移应为 9"))
        && expect(toDiscreteInputOffset(DiscreteInputs::OutfeedReady) == 81, QStringLiteral("下料就绪离散输入偏移应为 81"))
        && expect(toInputRegisterOffset(InputRegisters::CurrentAlarmCode) == 2, QStringLiteral("当前报警寄存器偏移应为 2"))
        && expect(toHoldingRegisterOffset(HoldingRegisters::SimFaultCode) == 10, QStringLiteral("模拟故障码保持寄存器偏移应为 10"));
}

bool tableExists(const QString& tableName)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT name FROM sqlite_master WHERE type = 'table' AND name = :name"));
    query.bindValue(QStringLiteral(":name"), tableName);
    return query.exec() && query.next();
}

bool testDatabaseSchema()
{
    QTemporaryDir tempDir(QDir::current().filePath(QStringLiteral("upkun-test-db-XXXXXX")));
    if (!expect(tempDir.isValid(), QStringLiteral("无法创建测试数据库目录"))) {
        return false;
    }

    const QString databasePath = QDir(tempDir.path()).filePath(QStringLiteral("app.sqlite3"));
    upkun::storage::DatabaseManager databaseManager;
    QString errorMessage;
    if (!expect(databaseManager.open(databasePath, &errorMessage), QStringLiteral("数据库初始化失败：%1").arg(errorMessage))) {
        return false;
    }

    const bool ok = expect(tableExists(QStringLiteral("alarms")), QStringLiteral("缺少 alarms 表"))
        && expect(tableExists(QStringLiteral("users")), QStringLiteral("缺少 users 表"))
        && expect(tableExists(QStringLiteral("recipes")), QStringLiteral("缺少 recipes 表"))
        && expect(tableExists(QStringLiteral("batches")), QStringLiteral("缺少 batches 表"))
        && expect(tableExists(QStringLiteral("trend_samples")), QStringLiteral("缺少 trend_samples 表"));

    {
        auto db = QSqlDatabase::database();
        db.close();
    }
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    return ok;
}

bool testModbusFaultInjection()
{
    upkun::simulator::SimulatedModbusServer server;
    QString errorMessage;
    if (!expect(server.start(QHostAddress::LocalHost, 0, &errorMessage), QStringLiteral("模拟器启动失败：%1").arg(errorMessage))) {
        return false;
    }

    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, server.serverPort());
    if (!expect(socket.waitForConnected(1000), QStringLiteral("测试客户端连接模拟器失败"))) {
        server.stop();
        return false;
    }
    QCoreApplication::processEvents();

    QByteArray response;
    const auto writeFaultCode = modbusRequest(1, 0x06, upkun::device::modbus::toHoldingRegisterOffset(upkun::device::modbus::HoldingRegisters::SimFaultCode), 9001);
    const auto triggerFault = modbusRequest(2, 0x05, upkun::device::modbus::toCoilOffset(upkun::device::modbus::Coils::CmdSimFault), 0xff00);
    const auto readAlarm = modbusRequest(3, 0x04, upkun::device::modbus::toInputRegisterOffset(upkun::device::modbus::InputRegisters::CurrentAlarmCode), 1);

    bool ok = sendAndExpectFunction(&socket, writeFaultCode, 0x06, nullptr)
        && sendAndExpectFunction(&socket, triggerFault, 0x05, nullptr)
        && sendAndExpectFunction(&socket, readAlarm, 0x04, &response)
        && expect(response.size() >= 11, QStringLiteral("报警码读取响应长度不足"))
        && expect(readU16(response, 9) == 9001, QStringLiteral("模拟故障报警码应为 9001"));

    socket.disconnectFromHost();
    server.stop();
    return ok;
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    const bool ok = testPointMap()
        && testDatabaseSchema()
        && testModbusFaultInjection();

    if (!ok) {
        return 1;
    }

    QTextStream(stdout) << "Regression tests passed." << Qt::endl;
    return 0;
}
