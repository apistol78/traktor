/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Online/Steam/SteamAchievements.h"
#include "Online/Steam/SteamSessionManager.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamAchievements", SteamAchievements, IAchievementsProvider)

SteamAchievements::SteamAchievements(SteamSessionManager* sessionManager, const std::list< std::wstring >& achievementIds)
:	m_sessionManager(sessionManager)
,	m_callbackAchievementStored(this, &SteamAchievements::OnAchievementStored)
{
	m_achievementIds.insert(achievementIds.begin(), achievementIds.end());
}

bool SteamAchievements::enumerate(std::map< std::wstring, bool >& outAchievements)
{
	bool haveStats = m_sessionManager->waitForStats();
	for (const auto& id : m_achievementIds)
	{
		bool achieved = false;
		if (haveStats)
		{
			if (!SteamUserStats()->GetAchievement(wstombs(id).c_str(), &achieved))
			{
				log::error << L"Unable to enumerate achievements; Achievement \"" << id << L"\" not available" << Endl;
				continue;
			}
		}
		outAchievements.insert(std::make_pair(
			id,
			achieved
		));
	}
	return true;
}

bool SteamAchievements::set(const std::wstring& achievementId, bool reward)
{
	if (!m_sessionManager->waitForStats())
		return false;

	if (reward)
	{
		if (!SteamUserStats()->SetAchievement(wstombs(achievementId).c_str()))
			return false;
	}
	else
	{
		if (!SteamUserStats()->ClearAchievement(wstombs(achievementId).c_str()))
			return false;
	}

	m_sessionManager->storeStats();
	return true;
}

void SteamAchievements::OnAchievementStored(UserAchievementStored_t* pCallback)
{
}

	}
}
