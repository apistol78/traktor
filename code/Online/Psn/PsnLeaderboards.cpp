#include "Online/Psn/PsnLeaderboards.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.PsnLeaderboards", PsnLeaderboards, ILeaderboardsProvider)

bool PsnLeaderboards::enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards)
{
	return true;
}

bool PsnLeaderboards::create(const std::wstring& leaderboardId, LeaderboardData& outLeaderboard)
{
	return false;
}

bool PsnLeaderboards::set(const uint64_t handle, int32_t score)
{
	return false;
}

bool PsnLeaderboards::getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores)
{
	return false;
}

bool PsnLeaderboards::getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores)
{
	return false;
}

	}
}
