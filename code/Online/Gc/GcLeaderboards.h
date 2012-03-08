#ifndef traktor_online_GcLeaderboards_H
#define traktor_online_GcLeaderboards_H

#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace online
	{

class GcLeaderboards : public ILeaderboardsProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards);

	virtual bool set(const uint64_t handle, int32_t score);
};

	}
}

#endif	// traktor_online_GcLeaderboards_H
