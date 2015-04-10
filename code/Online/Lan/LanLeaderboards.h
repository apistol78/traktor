#ifndef traktor_online_LanLeaderboards_H
#define traktor_online_LanLeaderboards_H

#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace online
	{

class LanLeaderboards : public ILeaderboardsProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards);

	virtual bool set(uint64_t handle, int32_t score);

	virtual bool getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores);

	virtual bool getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores);
};

	}
}

#endif	// traktor_online_LanLeaderboards_H
