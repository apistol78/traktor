#ifndef traktor_online_FriendUserSteam_H
#define traktor_online_FriendUserSteam_H

#include <steam/steam_api.h>
#include "Online/IUser.h"

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

	virtual bool sendMessage(const std::wstring& message) const;

private:
	CSteamID m_steamId;
};

	}
}

#endif	// traktor_online_FriendUserSteam_H
