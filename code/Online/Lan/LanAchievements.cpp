/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
