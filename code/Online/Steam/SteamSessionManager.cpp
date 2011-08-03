#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
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
	{ L"japanese", L"jp" },
	{ L"koreana", L"kr" },
	{ L"spanish", L"es" },
	{ L"russian", L"ru" },
	{ L"polish", L"pl" },
	{ L"portuguese", L"pt" },
	{ L"schinese", L"ch" },
	{ L"swedish", L"sv" }

/*
From steamworks mailinglist. 

Chris Boyd @ valve:
GetCurrentGameLanguage() can return the following:

english
german
french
italian
koreana
spanish
schinese
tchinese
russian
thai
japanese
portuguese
polish
danish
dutch
finnish
norwegian
swedish
hungarian
czech
romanian
turkish
brazilian
Bulgarian

Maybe we should map to IETF to avoid conflicts? http://en.wikipedia.org/wiki/IETF_language_tag

*/


};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamSessionManager", SteamSessionManager, ISessionManagerProvider)

SteamSessionManager::SteamSessionManager()
:	m_requireUserAttention(false)
,	m_requestedStats(false)
,	m_receivedStats(false)
,	m_receivedStatsSucceeded(false)
,	m_requestAttempts(0)
,	m_maxRequestAttempts(0)
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

	if (!SteamUser()->BLoggedOn())
		log::warning << L"Steam running in offline mode; Some features will be disabled or postponed until connected" << Endl;

	m_maxRequestAttempts = desc.requestAttempts;

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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_requestedStats)
	{
		if (SteamUser()->BLoggedOn())
		{
			if (SteamUserStats()->RequestCurrentStats())
			{
				log::debug << L"Steam; Current stats requested" << Endl;
				m_requestedStats = true;
			}
		}
	}
	else if (m_receivedStats)
	{
		if (!m_receivedStatsSucceeded)
		{
			if (++m_requestAttempts < m_maxRequestAttempts)
			{
				log::error << L"Steam; Failed to request current stats, retrying later..." << Endl;
				m_receivedStats = false;
				m_requestedStats = false;
			}
			else if (m_requestAttempts == m_maxRequestAttempts)
				log::error << L"Steam; Failed to request current stats, will not be available" << Endl;
		}
		else
			m_requestAttempts = 0;
	}

	SteamAPI_RunCallbacks();
	return true;
}

std::wstring SteamSessionManager::getLanguageCode() const
{
	const char* allLanguages = SteamApps()->GetAvailableGameLanguages();
	log::info << L"Steam; Available languages: " << mbstows(allLanguages) << Endl;
	const char* language = SteamApps()->GetCurrentGameLanguage();
	if (!language)
		return L"";

	for (uint32_t i = 0; i < sizeof_array(c_languageCodes); ++i)
	{
		if (compareIgnoreCase< std::wstring >(mbstows(language), c_languageCodes[i].steam) == 0)
			return c_languageCodes[i].code;
	}

	log::error << L"Steam; Unable to map language \"" << mbstows(language) << L"\" to ISO 639-1 code" << Endl;
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
	if (!m_receivedStats)
	{
		if (!SteamUser()->BLoggedOn())
			return false;

		log::info << L"Steam; Waiting for stats..." << Endl;
		Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
		while (!m_receivedStats)
		{
			if (!update())
				return false;

			if (currentThread)
				currentThread->sleep(100);

			if (!SteamUser()->BLoggedOn())
				break;
		}
	}
	return m_receivedStatsSucceeded;
}

bool SteamSessionManager::storeStats()
{
	return SteamUserStats()->StoreStats();
}

void SteamSessionManager::OnUserStatsReceived(UserStatsReceived_t* pCallback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_receivedStats = true;
	m_receivedStatsSucceeded = (pCallback->m_eResult == k_EResultOK);
	log::debug << L"Steam; Receieved stats (eResult = " << getSteamError(pCallback->m_eResult) << L")" << Endl;
}

void SteamSessionManager::OnOverlayActivated(GameOverlayActivated_t* pCallback)
{
	m_requireUserAttention = (pCallback->m_bActive != 0);
}

	}
}
