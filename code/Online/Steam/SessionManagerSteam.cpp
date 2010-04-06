#include <steam/steam_api.h>
#include "Core/Log/Log.h"
#include "Online/Steam/CurrentUserSteam.h"
#include "Online/Steam/SessionManagerSteam.h"
#include "Online/Steam/SessionSteam.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.SessionManagerSteam", 0, SessionManagerSteam, ISessionManager)

bool SessionManagerSteam::create()
{
	if (!SteamAPI_Init())
		return false;

	m_currentUser = new CurrentUserSteam();
	return true;
}

void SessionManagerSteam::destroy()
{
	m_currentUser = 0;
	SteamAPI_Shutdown();
}

bool SessionManagerSteam::getAvailableUsers(RefArray< IUser >& outUsers)
{
	return false;
}

Ref< IUser > SessionManagerSteam::getCurrentUser()
{
	return m_currentUser;
}

Ref< ISession > SessionManagerSteam::createSession(IUser* user)
{
	T_ASSERT (user == m_currentUser);
	return new SessionSteam(m_currentUser);
}

bool SessionManagerSteam::update()
{
	SteamAPI_RunCallbacks();
	return true;
}

	}
}
