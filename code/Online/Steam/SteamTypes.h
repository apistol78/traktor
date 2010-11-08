#ifndef traktor_online_SteamTypes_H
#define traktor_online_SteamTypes_H

#include <steam/steam_api.h>

namespace traktor
{
	namespace online
	{

struct SteamCreateDesc
{
	const wchar_t** achievementIds;
	const wchar_t** leaderboardIds;
	const wchar_t** statIds;
};

std::wstring getSteamError(EResult result);

	}
}

#endif	// traktor_online_SteamTypes_H
