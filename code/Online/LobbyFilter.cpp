#include "Online/LobbyFilter.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LobbyFilter", LobbyFilter, Object)

LobbyFilter::LobbyFilter()
:	m_distance(DtInfinity)
,	m_slots(0)
,	m_count(-1)
{
}

void LobbyFilter::addComparison(const std::wstring& key, const std::wstring& value, ComparisonType comparison)
{
	StringComparison scr;
	scr.key = key;
	scr.value = value;
	scr.comparison = comparison;
	m_stringComparisons.push_back(scr);
}

void LobbyFilter::addComparison(const std::wstring& key, int32_t value, ComparisonType comparison)
{
	NumberComparison ncr;
	ncr.key = key;
	ncr.value = value;
	ncr.comparison = comparison;
	m_numberComparisons.push_back(ncr);
}

void LobbyFilter::setDistance(DistanceType distance)
{
	m_distance = distance;
}

void LobbyFilter::setSlots(int32_t slots)
{
	m_slots = slots;
}

void LobbyFilter::setCount(int32_t count)
{
	m_count = count;
}

	}
}
