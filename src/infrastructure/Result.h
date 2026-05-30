#pragma once

#include <QString>
#include <utility>

namespace upkun::infrastructure {

struct Result {
    bool ok = true;
    QString message;

    static Result success()
    {
        return {};
    }

    static Result failure(QString message)
    {
        return {false, std::move(message)};
    }
};

} // namespace upkun::infrastructure
