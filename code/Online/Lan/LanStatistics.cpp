#include "Online/Lan/LanStatistics.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LanStatistics", LanStatistics, IStatisticsProvider)

bool LanStatistics::enumerate(std::map< std::wstring, int32_t >& outStats)
{
	return true;
}

bool LanStatistics::set(const std::wstring& statId, int32_t value)
{
	return true;
}

	}
}
