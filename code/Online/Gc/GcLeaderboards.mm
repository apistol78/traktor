#include "Online/Gc/GcLeaderboards.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcLeaderboards", GcLeaderboards, ILeaderboardsProvider)

bool GcLeaderboards::enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards)
{
	return false;
}

bool GcLeaderboards::set(const uint64_t handle, int32_t score)
{
	return false;
}

	}
}
