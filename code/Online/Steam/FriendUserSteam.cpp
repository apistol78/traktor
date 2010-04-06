#include "Core/Misc/TString.h"
#include "Online/Steam/FriendUserSteam.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.FriendUserSteam", FriendUserSteam, IUser)

FriendUserSteam::FriendUserSteam(CSteamID steamId)
:	m_steamId(steamId)
{
}

std::wstring FriendUserSteam::getName() const
{
	const char* persona = SteamFriends()->GetFriendPersonaName(m_steamId);
	return mbstows(persona);
}

bool FriendUserSteam::getFriends(RefArray< IUser >& outFriends) const
{
	return false;
}

bool FriendUserSteam::getAchievements(RefArray< IAchievement >& outAchievements) const
{
	return false;
}

bool FriendUserSteam::rewardAchievement(IAchievement* achievement)
{
	return false;
}

bool FriendUserSteam::sendMessage(const std::wstring& message) const
{
	return false;
}

	}
}
