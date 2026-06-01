#pragma once

#include "domain/DeviceTypes.h"
#include "domain/Recipe.h"

#include <QObject>
#include <QString>

namespace upkun::device {

class IDeviceClient : public QObject {
    Q_OBJECT

public:
    explicit IDeviceClient(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

public slots:
    virtual void connectToDevice(const upkun::domain::DeviceConnectionConfig& config) = 0;
    virtual void disconnectFromDevice() = 0;
    virtual void sendCommand(upkun::domain::DeviceCommand command) = 0;
    virtual void writeRecipe(const upkun::domain::RecipeParameters& recipe) = 0;

signals:
    void connectionChanged(upkun::domain::ConnectionState state);
    void snapshotUpdated(upkun::domain::DeviceSnapshot snapshot);
    void commandFinished(upkun::domain::DeviceCommand command, bool ok, QString message);
    void errorOccurred(upkun::domain::DeviceError error);
    void diagnosticsUpdated(upkun::domain::CommunicationDiagnostics diagnostics);
};

} // namespace upkun::device
