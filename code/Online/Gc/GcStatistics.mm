#include "Online/Gc/GcStatistics.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcStatistics", GcStatistics, IStatisticsProvider)

bool GcStatistics::enumerate(std::map< std::wstring, int32_t >& outStats)
{
	return false;
}

bool GcStatistics::set(const std::wstring& statId, int32_t value)
{
	return false;
}

	}
}
