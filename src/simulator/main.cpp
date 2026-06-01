#include "simulator/SimulatedModbusServer.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QHostAddress>
#include <QTextStream>

#ifndef UPKUN_APP_VERSION
#define UPKUN_APP_VERSION "0.17.0"
#endif

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("upkun-simulator"));
    QCoreApplication::setApplicationVersion(QStringLiteral(UPKUN_APP_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Upkun 包装产线 Modbus TCP 模拟器"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption hostOption(QStringLiteral("host"), QStringLiteral("监听地址"), QStringLiteral("host"), QStringLiteral("127.0.0.1"));
    QCommandLineOption portOption(QStringLiteral("port"), QStringLiteral("监听端口"), QStringLiteral("port"), QStringLiteral("1502"));
    parser.addOption(hostOption);
    parser.addOption(portOption);
    parser.process(app);

    const QString host = parser.value(hostOption);
    bool portOk = false;
    const quint16 port = static_cast<quint16>(parser.value(portOption).toUShort(&portOk));
    if (!portOk || port == 0) {
        QTextStream(stderr) << "Invalid simulator port." << Qt::endl;
        return 2;
    }

    upkun::simulator::SimulatedModbusServer server;
    QString errorMessage;
    if (!server.start(QHostAddress(host), port, &errorMessage)) {
        QTextStream(stderr) << "Failed to start simulator: " << errorMessage << Qt::endl;
        return 1;
    }

    QTextStream(stdout) << "Upkun simulator listening on " << host << ":" << port << Qt::endl;
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &server, &upkun::simulator::SimulatedModbusServer::stop);
    return app.exec();
}
