/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
	for (std::set< std::wstring >::const_iterator i = m_achievementIds.begin(); i != m_achievementIds.end(); ++i)
	{
		bool achieved = false;
		if (haveStats)
		{
			if (!SteamUserStats()->GetAchievement(wstombs(*i).c_str(), &achieved))
			{
				log::error << L"Unable to enumerate achievements; Achievement \"" << *i << L"\" not available" << Endl;
				continue;
			}
		}
		outAchievements.insert(std::make_pair(
			*i,
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
