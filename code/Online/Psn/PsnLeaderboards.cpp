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

bool PsnLeaderboards::set(const uint64_t handle, int32_t score)
{
	return false;
}

	}
}
