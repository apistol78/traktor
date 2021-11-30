#include "Online/LobbyFilter.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LobbyFilter", LobbyFilter, Object)

void LobbyFilter::addComparison(const std::wstring& key, const std::wstring& value, Comparison comparison)
{
	m_stringComparisons.push_back({ key, value, comparison });
}

void LobbyFilter::addComparison(const std::wstring& key, int32_t value, Comparison comparison)
{
	m_numberComparisons.push_back({ key, value, comparison });
}

void LobbyFilter::setDistance(Distance distance)
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
