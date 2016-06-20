#include "Online/Lan/LanLeaderboards.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LanLeaderboards", LanLeaderboards, ILeaderboardsProvider)

bool LanLeaderboards::enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards)
{
	return true;
}

bool LanLeaderboards::create(const std::wstring& leaderboardId, LeaderboardData& outLeaderboard)
{
	return false;
}

bool LanLeaderboards::set(const uint64_t handle, int32_t score)
{
	return true;
}

bool LanLeaderboards::getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores)
{
	return false;
}

bool LanLeaderboards::getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores)
{
	return false;
}

	}
}
