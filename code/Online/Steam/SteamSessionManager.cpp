#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Online/Steam/SteamAchievements.h"
#include "Online/Steam/SteamLeaderboards.h"
#include "Online/Steam/SteamSaveData.h"
#include "Online/Steam/SteamStatistics.h"
#include "Online/Steam/SteamSessionManager.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

const struct { const wchar_t* steam; const wchar_t* code; } c_languageCodes[] =
{
	{ L"english", L"en" },
	{ L"french", L"fr" },
	{ L"german", L"de" },
	{ L"italian", L"it" },
	{ L"spanish", L"es" },
	{ L"russian", L"ru" },
	{ L"polish", L"pl" },
	{ L"portuguese", L"pt" },
	{ L"swedish", L"sv" }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamSessionManager", SteamSessionManager, ISessionManagerProvider)

SteamSessionManager::SteamSessionManager()
:	m_requireUserAttention(false)
,	m_requestedStats(false)
,	m_receivedStats(false)
,	m_receivedStatsSucceeded(false)
,	m_callbackUserStatsReceived(this, &SteamSessionManager::OnUserStatsReceived)
,	m_callbackOverlay(this, &SteamSessionManager::OnOverlayActivated)
{
}

bool SteamSessionManager::create(const SteamCreateDesc& desc)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	
	bool result = false;
	for (int32_t i = 0; i < 10; ++i)
	{
		if (SteamAPI_Init())
		{
			result = true;
			break;
		}
		log::error << L"Steam API failed to initialize; retrying..." << Endl;
		if (currentThread)
			currentThread->sleep(500);
	}
	if (!result)
	{
		log::error << L"Session manager failed; Unable to initialize Steam API" << Endl;
		return false;
	}

	m_achievements = new SteamAchievements(this, desc.achievementIds);
	m_leaderboards = new SteamLeaderboards(this, desc.leaderboardIds);
	m_saveData = new SteamSaveData();
	m_statistics = new SteamStatistics(this, desc.statIds);

	return true;
}

void SteamSessionManager::destroy()
{
	m_statistics = 0;
	m_saveData = 0;
	m_leaderboards = 0;
	m_achievements = 0;

	SteamAPI_Shutdown();
}

bool SteamSessionManager::update()
{
	if (!m_requestedStats)
	{
		if (SteamUser()->BLoggedOn())
		{
			if (SteamUserStats()->RequestCurrentStats())
				m_requestedStats = true;
		}
	}
	else if (m_receivedStats)
	{
		if (!m_receivedStatsSucceeded)
			m_requestedStats = false;
	}

	SteamAPI_RunCallbacks();
	return true;
}

std::wstring SteamSessionManager::getLanguageCode() const
{
	const char* language = SteamApps()->GetCurrentGameLanguage();
	if (!language)
		return L"";

	for (uint32_t i = 0; i < sizeof_array(c_languageCodes); ++i)
	{
		if (compareIgnoreCase< std::wstring >(mbstows(language), c_languageCodes[i].steam) == 0)
			return c_languageCodes[i].code;
	}

	log::error << L"Unable to map language \"" << mbstows(language) << L"\" to ISO 639-1 code" << Endl;
	return L"";
}

bool SteamSessionManager::isConnected() const
{
	return SteamUser()->BLoggedOn();
}

bool SteamSessionManager::requireUserAttention() const
{
	return m_requireUserAttention;
}

Ref< IAchievementsProvider > SteamSessionManager::getAchievements() const
{
	return m_achievements;
}

Ref< ILeaderboardsProvider > SteamSessionManager::getLeaderboards() const
{
	return m_leaderboards;
}

Ref< ISaveDataProvider > SteamSessionManager::getSaveData() const
{
	return m_saveData;
}

Ref< IStatisticsProvider > SteamSessionManager::getStatistics() const
{
	return m_statistics;
}

bool SteamSessionManager::waitForStats()
{
	while (!m_receivedStats)
	{
		if (!update())
			return false;

		ThreadManager::getInstance().getCurrentThread()->sleep(200);
	}
	return m_receivedStatsSucceeded;
}

void SteamSessionManager::OnUserStatsReceived(UserStatsReceived_t* pCallback)
{
	m_receivedStats = true;
	m_receivedStatsSucceeded = (pCallback->m_eResult == k_EResultOK);
}

void SteamSessionManager::OnOverlayActivated(GameOverlayActivated_t* pCallback)
{
	m_requireUserAttention = (pCallback->m_bActive != 0);
}

	}
}
