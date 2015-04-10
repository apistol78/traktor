#include "Online/Lan/LanAchievements.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LanAchievements", LanAchievements, IAchievementsProvider)

bool LanAchievements::enumerate(std::map< std::wstring, bool >& outAchievements)
{
	return true;
}

bool LanAchievements::set(const std::wstring& achievementId, bool reward)
{
	return true;
}

	}
}
