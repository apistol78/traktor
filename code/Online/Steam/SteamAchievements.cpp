#include "Core/Misc/TString.h"
#include "Online/Steam/SteamAchievements.h"
#include "Online/Steam/SteamSessionManager.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamAchievements", SteamAchievements, IAchievementsProvider)

SteamAchievements::SteamAchievements(SteamSessionManager* sessionManager, const wchar_t** achievementIds)
:	m_sessionManager(sessionManager)
,	m_storeAchievement(false)
,	m_callbackAchievementStored(this, &SteamAchievements::OnAchievementStored)
{
	for (const wchar_t** achievementId = achievementIds; *achievementId; ++achievementId)
		m_achievementIds.insert(*achievementId);
}

bool SteamAchievements::enumerate(std::map< std::wstring, bool >& outAchievements)
{
	if (!m_sessionManager->waitForStats())
		return false;

	for (std::set< std::wstring >::const_iterator i = m_achievementIds.begin(); i != m_achievementIds.end(); ++i)
	{
		bool achieved = false;
		if (!SteamUserStats()->GetAchievement(wstombs(*i).c_str(), &achieved))
			return false;

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

	m_storeAchievement = true;
	return true;
}

void SteamAchievements::OnAchievementStored(UserAchievementStored_t* pCallback)
{
	m_storeAchievement = false;
}

	}
}
