#pragma once

#include "domain/DeviceTypes.h"

#include <QString>

namespace upkun::storage {

class TrendRepository final {
public:
    bool appendSample(const upkun::domain::DeviceSnapshot& snapshot, QString* errorMessage = nullptr);
    bool exportCsv(const QString& filePath, QString* errorMessage = nullptr) const;
};

} // namespace upkun::storage
