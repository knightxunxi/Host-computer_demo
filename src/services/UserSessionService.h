#pragma once

#include "domain/User.h"

#include <QObject>
#include <optional>

namespace upkun::services {

class UserSessionService final : public QObject {
    Q_OBJECT

public:
    explicit UserSessionService(QObject* parent = nullptr);

    std::optional<upkun::domain::User> currentUser() const;
    void setCurrentUser(upkun::domain::User user);
    void clear();

signals:
    void currentUserChanged();

private:
    std::optional<upkun::domain::User> m_currentUser;
};

} // namespace upkun::services
