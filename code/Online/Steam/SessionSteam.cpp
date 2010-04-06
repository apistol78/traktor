#include <steam/steam_api.h>
#include "Online/Steam/CurrentUserSteam.h"
#include "Online/Steam/SessionSteam.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SessionSteam", SessionSteam, ISession)

SessionSteam::SessionSteam(CurrentUserSteam* user)
:	m_user(user)
{
}

void SessionSteam::destroy()
{
}

bool SessionSteam::isConnected() const
{
	return SteamUser()->BLoggedOn();
}

Ref< IUser > SessionSteam::getUser()
{
	return m_user;
}

bool SessionSteam::getAvailableAchievements(RefArray< IAchievement >& outAchievements) const
{
	return false;
}

Ref< ISaveGame > SessionSteam::createSaveGame(const std::wstring& name, ISerializable* attachment)
{
	return 0;
}

bool SessionSteam::getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const
{
	return false;
}

	}
}
