#pragma once

#include <QString>

namespace upkun::domain {

struct RecipeParameters {
    int id = 0;
    QString name = QStringLiteral("默认配方");
    int targetSpeed = 60;
    int fillVolumeMl = 500;
    int fillTimeMs = 1000;
    int cappingTorqueCentinewtonMeter = 125;
    int weightMinGram = 480;
    int weightMaxGram = 520;
    int labelMode = 1;
    int batchTargetCount = 1000;
    int simulationQualityRate = 98;
};

} // namespace upkun::domain
