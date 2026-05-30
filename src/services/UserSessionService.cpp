#include "services/UserSessionService.h"

#include <utility>

namespace upkun::services {

UserSessionService::UserSessionService(QObject* parent)
    : QObject(parent)
{
}

std::optional<upkun::domain::User> UserSessionService::currentUser() const
{
    return m_currentUser;
}

void UserSessionService::setCurrentUser(upkun::domain::User user)
{
    m_currentUser = std::move(user);
    emit currentUserChanged();
}

void UserSessionService::clear()
{
    m_currentUser.reset();
    emit currentUserChanged();
}

} // namespace upkun::services
