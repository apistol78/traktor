#include "Online/Psn/PsnStatistics.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.PsnStatistics", PsnStatistics, IStatisticsProvider)

bool PsnStatistics::enumerate(std::map< std::wstring, float >& outStats)
{
	return true;
}

bool PsnStatistics::set(const std::wstring& statId, float value)
{
	return false;
}

	}
}
