#include <np.h>
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
	int32_t state;
	if (sceNpManagerGetStatus(&state) != 0)
		return false;

	return state == SCE_NP_MANAGER_STATUS_ONLINE;
}

Ref< IUser > SessionPsn::getUser()
{
	return m_user;
}

bool SessionPsn::rewardAchievement(const std::wstring& achievementId)
{
	return true;
}

bool SessionPsn::withdrawAchievement(const std::wstring& achievementId)
{
	return false;
}

bool SessionPsn::haveAchievement(const std::wstring& achievementId)
{
	return false;
}

Ref< ILeaderboard > SessionPsn::getLeaderboard(const std::wstring& id)
{
	return 0;
}

bool SessionPsn::setStatValue(const std::wstring& statId, float value)
{
	return false;
}

bool SessionPsn::getStatValue(const std::wstring& statId, float& outValue)
{
	return false;
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
