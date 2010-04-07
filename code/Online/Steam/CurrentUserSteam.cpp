#include <steam/steam_api.h>
#include "Core/Misc/TString.h"
#include "Online/Steam/CurrentUserSteam.h"
#include "Online/Steam/FriendUserSteam.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.CurrentUserSteam", CurrentUserSteam, IUser)

CurrentUserSteam::CurrentUserSteam()
{
}

std::wstring CurrentUserSteam::getName() const
{
	const char* persona = SteamFriends()->GetPersonaName();
	return mbstows(persona);
}

bool CurrentUserSteam::getFriends(RefArray< IUser >& outFriends) const
{
	int friendCount = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
	for (int friendIndex = 0; friendIndex < friendCount; ++friendIndex)
	{
		CSteamID friendId = SteamFriends()->GetFriendByIndex(friendCount, k_EFriendFlagImmediate);
		if (friendId.IsValid())
			outFriends.push_back(new FriendUserSteam(friendId));
	}
	return true;
}

bool CurrentUserSteam::sendMessage(const std::wstring& message) const
{
	return false;
}

	}
}
