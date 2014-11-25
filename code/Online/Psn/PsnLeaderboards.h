#ifndef traktor_online_PsnLeaderboards_H
#define traktor_online_PsnLeaderboards_H

#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace online
	{

class PsnLeaderboards : public ILeaderboardsProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards);

	virtual bool set(const uint64_t handle, int32_t score);

	virtual bool getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores);

	virtual bool getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores);
};

	}
}

#endif	// traktor_online_PsnLeaderboards_H
