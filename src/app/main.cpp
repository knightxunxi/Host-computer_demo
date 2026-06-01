#include "app/MainWindow.h"
#include "domain/DeviceTypes.h"
#include "domain/Recipe.h"

#include <QApplication>
#include <QCoreApplication>
#include <QMetaType>

#ifndef UPKUN_APP_VERSION
#define UPKUN_APP_VERSION "0.17.0"
#endif

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName(QStringLiteral("Upkun HMI"));
    QCoreApplication::setApplicationVersion(QStringLiteral(UPKUN_APP_VERSION));
    QCoreApplication::setOrganizationName(QStringLiteral("Upkun"));

    qRegisterMetaType<upkun::domain::DeviceCommand>("upkun::domain::DeviceCommand");
    qRegisterMetaType<upkun::domain::DeviceSnapshot>("upkun::domain::DeviceSnapshot");
    qRegisterMetaType<upkun::domain::ConnectionState>("upkun::domain::ConnectionState");
    qRegisterMetaType<upkun::domain::CommunicationDiagnostics>("upkun::domain::CommunicationDiagnostics");
    qRegisterMetaType<upkun::domain::DeviceError>("upkun::domain::DeviceError");
    qRegisterMetaType<upkun::domain::RecipeParameters>("upkun::domain::RecipeParameters");

    upkun::app::MainWindow window;
    window.show();

    return app.exec();
}
