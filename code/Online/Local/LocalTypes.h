#ifndef traktor_online_LocalTypes_H
#define traktor_online_LocalTypes_H

namespace traktor
{
	namespace online
	{

struct LocalCreateDesc
{
	const wchar_t* dbName;
	const wchar_t** achievementIds;
	const wchar_t** leaderboardIds;
	const wchar_t** statIds;
};

	}
}

#endif	// traktor_online_LocalTypes_H
