/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
