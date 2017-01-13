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
	virtual bool enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards) T_OVERRIDE T_FINAL;

	virtual bool create(const std::wstring& leaderboardId, LeaderboardData& outLeaderboard) T_OVERRIDE T_FINAL;

	virtual bool set(uint64_t handle, int32_t score) T_OVERRIDE T_FINAL;

	virtual bool getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores) T_OVERRIDE T_FINAL;

	virtual bool getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_online_LanLeaderboards_H
