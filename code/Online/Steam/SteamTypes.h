#ifndef traktor_online_SteamTypes_H
#define traktor_online_SteamTypes_H

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

	}
}

#endif	// traktor_online_SteamTypes_H
