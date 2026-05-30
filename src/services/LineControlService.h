#pragma once

#include "domain/DeviceTypes.h"

#include <QObject>
#include <QString>

namespace upkun::services {

class LineControlService final : public QObject {
    Q_OBJECT

public:
    explicit LineControlService(QObject* parent = nullptr);

    bool canStart(const upkun::domain::DeviceSnapshot& snapshot, QString* reason = nullptr) const;

signals:
    void commandRequested(upkun::domain::DeviceCommand command);
};

} // namespace upkun::services
