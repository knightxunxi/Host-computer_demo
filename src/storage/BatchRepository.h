#pragma once

#include "domain/Batch.h"

#include <QString>
#include <QStringList>
#include <QVector>
#include <optional>

namespace upkun::storage {

class BatchRepository final {
public:
    bool startBatch(upkun::domain::ProductionBatch* batch, QString* errorMessage = nullptr);
    bool finishActiveBatch(int batchId, int totalCount, int goodCount, int badCount, QString* errorMessage = nullptr);
    std::optional<upkun::domain::ProductionBatch> activeBatch() const;
    QVector<QStringList> recentRows(int limit = 50) const;
};

} // namespace upkun::storage
