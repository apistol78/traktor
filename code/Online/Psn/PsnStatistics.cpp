#include "Online/Psn/PsnStatistics.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.PsnStatistics", PsnStatistics, IStatisticsProvider)

bool PsnStatistics::enumerate(std::map< std::wstring, int32_t >& outStats)
{
	return true;
}

bool PsnStatistics::set(const std::wstring& statId, int32_t value)
{
	return false;
}

	}
}
