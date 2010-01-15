#include "Core/System/OS.h"
#include "Online/Local/SessionLocal.h"
#include "Online/Local/SessionManagerLocal.h"
#include "Online/Local/UserLocal.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.SessionManagerLocal", 0, SessionManagerLocal, ISessionManager)

SessionManagerLocal::SessionManagerLocal()
{
	std::wstring user = OS::getInstance().getCurrentUser();
	m_currentUser = new UserLocal(user);
}

bool SessionManagerLocal::getAvailableUsers(RefArray< IUser >& outUsers)
{
	outUsers.push_back(m_currentUser);
	return true;
}

Ref< IUser > SessionManagerLocal::getCurrentUser()
{
	return m_currentUser;
}

Ref< ISession > SessionManagerLocal::createSession(IUser* user)
{
	T_ASSERT (user == m_currentUser);
	return new SessionLocal(checked_type_cast< UserLocal*, false >(user));
}

	}
}
