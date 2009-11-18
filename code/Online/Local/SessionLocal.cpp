#include "Online/Local/SessionLocal.h"
#include "Online/Local/UserLocal.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SessionLocal", SessionLocal, ISession)

SessionLocal::SessionLocal(UserLocal* user)
:	m_user(user)
{
}

void SessionLocal::destroy()
{
}

bool SessionLocal::isConnected() const
{
	return false;
}

Ref< IUser > SessionLocal::getUser()
{
	return m_user;
}

bool SessionLocal::getAvailableAchievements(RefArray< IAchievement >& outAchievements) const
{
	return true;
}

Ref< ISaveGame > SessionLocal::createSaveGame(const std::wstring& name, ISerializable* attachment)
{
	return 0;
}

bool SessionLocal::getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const
{
	return false;
}

	}
}
