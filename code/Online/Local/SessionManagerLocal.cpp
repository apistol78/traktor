#include "Online/Local/SessionManagerLocal.h"
#include "Online/Local/UserLocal.h"
#include "Online/Local/SessionLocal.h"
#include "Core/Heap/GcNew.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SessionManagerLocal", SessionManagerLocal, ISessionManager)

SessionManagerLocal::SessionManagerLocal()
:	m_currentUser(gc_new< UserLocal >(L"Local"))
{
}

bool SessionManagerLocal::getAvailableUsers(RefArray< IUser >& outUsers)
{
	outUsers.push_back(m_currentUser);
	return true;
}

IUser* SessionManagerLocal::getCurrentUser()
{
	return m_currentUser;
}

ISession* SessionManagerLocal::createSession(IUser* user)
{
	T_ASSERT (user == m_currentUser);
	return gc_new< SessionLocal >(checked_type_cast< UserLocal* >(user));
}

	}
}
