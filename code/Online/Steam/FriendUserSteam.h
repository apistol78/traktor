#ifndef traktor_online_FriendUserSteam_H
#define traktor_online_FriendUserSteam_H

#include <steam/steam_api.h>
#include "Online/IUser.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_STEAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

class ISteamUser;

namespace traktor
{
	namespace online
	{

class FriendUserSteam : public IUser
{
	T_RTTI_CLASS;

public:
	FriendUserSteam(CSteamID steamId);

	virtual std::wstring getName() const;

	virtual bool getFriends(RefArray< IUser >& outFriends) const;

	virtual bool getAchievements(RefArray< IAchievement >& outAchievements) const;

	virtual bool rewardAchievement(IAchievement* achievement);

	virtual bool sendMessage(const std::wstring& message) const;

private:
	CSteamID m_steamId;
};

	}
}

#endif	// traktor_online_FriendUserSteam_H
