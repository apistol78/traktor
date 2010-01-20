#include "Online/Psn/SessionPsn.h"
#include "Online/Psn/UserPsn.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SessionPsn", SessionPsn, ISession)

SessionPsn::SessionPsn(UserPsn* user)
:	m_user(user)
{
}

void SessionPsn::destroy()
{
}

bool SessionPsn::isConnected() const
{
	return false;
}

Ref< IUser > SessionPsn::getUser()
{
	return m_user;
}

bool SessionPsn::getAvailableAchievements(RefArray< IAchievement >& outAchievements) const
{
	return true;
}

Ref< ISaveGame > SessionPsn::createSaveGame(const std::wstring& name, ISerializable* attachment)
{
	return 0;
}

bool SessionPsn::getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const
{
	return false;
}

	}
}
