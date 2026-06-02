#pragma once

#include "device/IDeviceClient.h"

namespace upkun::device {

class ModbusRtuClient final : public IDeviceClient {
    Q_OBJECT

public:
    explicit ModbusRtuClient(QObject* parent = nullptr);

public slots:
    void connectToDevice(const upkun::domain::DeviceConnectionConfig& config) override;
    void disconnectFromDevice() override;
    void sendCommand(upkun::domain::DeviceCommand command) override;
    void writeRecipe(const upkun::domain::RecipeParameters& recipe) override;

private:
    void publishDiagnostics();
    void reportUnavailable(const QString& action);

    upkun::domain::DeviceConnectionConfig m_config;
    upkun::domain::CommunicationDiagnostics m_diagnostics;
};

} // namespace upkun::device
