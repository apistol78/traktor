#if defined(_WIN32)
#	include <windows.h>
#endif
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Net/Url.h"
#include "Online/Steam/SteamAchievements.h"
#include "Online/Steam/SteamGameConfiguration.h"
#include "Online/Steam/SteamLeaderboards.h"
#include "Online/Steam/SteamLocalSaveData.h"
#include "Online/Steam/SteamMatchMaking.h"
#include "Online/Steam/SteamCloudSaveData.h"
#include "Online/Steam/SteamSessionManager.h"
#include "Online/Steam/SteamStatistics.h"
#include "Online/Steam/SteamUser.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

/*!
 * \brief Language code translation table.
 * 
 * GetCurrentGameLanguage() can return the following:
 * 
 * english
 * german
 * french
 * italian
 * koreana
 * spanish
 * schinese
 * tchinese
 * russian
 * thai
 * japanese
 * portuguese
 * polish
 * danish
 * dutch
 * finnish
 * norwegian
 * swedish
 * hungarian
 * czech
 * romanian
 * turkish
 * brazilian
 * bulgarian
 * 
 * \todo
 * Maybe we should map to IETF to avoid conflicts? http://en.wikipedia.org/wiki/IETF_language_tag
 */
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
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.SteamSessionManager", 0, SteamSessionManager, ISessionManagerProvider)

SteamSessionManager::SteamSessionManager()
:	m_requireUserAttention(false)
,	m_requestedStats(false)
,	m_receivedStats(false)
,	m_receivedStatsSucceeded(false)
,	m_requestAttempts(0)
,	m_maxRequestAttempts(0)
,	m_updateGameCountTicks(0)
,	m_currentGameCount(0)
,	m_callbackUserStatsReceived(this, &SteamSessionManager::OnUserStatsReceived)
,	m_callbackOverlay(this, &SteamSessionManager::OnOverlayActivated)
,	m_callbackSessionRequest(this, &SteamSessionManager::OnP2PSessionRequest)
{
}

bool SteamSessionManager::create(const IGameConfiguration* configuration)
{
	const SteamGameConfiguration* gc = dynamic_type_cast< const SteamGameConfiguration* >(configuration);
	if (!gc)
		return false;

#if !defined(__APPLE__)
#	if defined(_WIN32)
	bool debuggerPresent = bool(IsDebuggerPresent() != FALSE);
#	else
	bool debuggerPresent = false;
#endif
	// Check if application needs to be restarted.
	if (!debuggerPresent && !gc->m_drmEnabled)
	{
		if (SteamAPI_RestartAppIfNecessary(gc->m_appId))
			return false;
	}
#endif

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
		log::error << L"Please restart Steam client and try again later" << Endl;
		return false;
	}

	if (!::SteamUser()->BLoggedOn())
		log::warning << L"Steam running in offline mode; Some features will be disabled or postponed until connected" << Endl;

	const char* allLanguages = SteamApps()->GetAvailableGameLanguages();
	if (allLanguages)
		log::info << L"Steam; Available languages: " << mbstows(allLanguages) << Endl;

	m_maxRequestAttempts = gc->m_requestAttempts;
	m_dlcIds = gc->m_dlcIds;

	SteamNetworking()->AllowP2PPacketRelay(gc->m_allowP2PRelay);

	m_achievements = new SteamAchievements(this, gc->m_achievementIds);
	m_leaderboards = new SteamLeaderboards(this, gc->m_leaderboardIds);
	m_matchMaking = new SteamMatchMaking(this);
	
	if (gc->m_cloudEnabled)
		m_saveData = new SteamCloudSaveData();
	else
		m_saveData = new SteamLocalSaveData();

	m_statistics = new SteamStatistics(this, gc->m_statsIds);
	m_user = new SteamUser();

	return true;
}

void SteamSessionManager::destroy()
{
	m_user = 0;
	m_statistics = 0;
	m_saveData = 0;
	m_matchMaking = 0;
	m_leaderboards = 0;
	m_achievements = 0;

	SteamAPI_Shutdown();
}

bool SteamSessionManager::update()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_requestedStats)
	{
		if (::SteamUser()->BLoggedOn())
		{
			if (SteamUserStats()->RequestCurrentStats())
			{
				T_DEBUG(L"Steam; Current stats requested");
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

	if (m_updateGameCountTicks-- <= 0)
	{
		SteamAPICall_t call = SteamUserStats()->GetNumberOfCurrentPlayers();
		m_callbackGameCount.Set(call, this, &SteamSessionManager::OnCurrentGameCount);
		m_updateGameCountTicks = 120;
	}

	if (m_matchMaking)
		m_matchMaking->update();

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

	log::error << L"Steam; Unable to map language \"" << mbstows(language) << L"\" to ISO 639-1 code" << Endl;
	return L"";
}

bool SteamSessionManager::isConnected() const
{
	return ::SteamUser()->BLoggedOn();
}

bool SteamSessionManager::requireUserAttention() const
{
	return m_requireUserAttention;
}

bool SteamSessionManager::haveDLC(const std::wstring& id) const
{
	std::map< std::wstring, uint32_t >::const_iterator i = m_dlcIds.find(id);
	if (i != m_dlcIds.end())
		return SteamApps()->BIsDlcInstalled(i->second);
	else
		return false;
}

bool SteamSessionManager::buyDLC(const std::wstring& id) const
{
	std::map< std::wstring, uint32_t >::const_iterator i = m_dlcIds.find(id);
	if (i != m_dlcIds.end())
	{
		SteamFriends()->ActivateGameOverlayToStore(i->second, k_EOverlayToStoreFlag_AddToCartAndShow);
		return true;
	}
	else
		return false;
}

bool SteamSessionManager::navigateUrl(const net::Url& url) const
{
	if (url.valid())
	{
		SteamFriends()->ActivateGameOverlayToWebPage(wstombs(url.getString()).c_str());
		return true;
	}
	else
		return false;
}

uint64_t SteamSessionManager::getCurrentUserHandle() const
{
	return ::SteamUser()->GetSteamID().ConvertToUint64();
}

bool SteamSessionManager::getFriends(std::vector< uint64_t >& outFriends, bool onlineOnly) const
{
	int friendCount = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
	if (friendCount < 0)
		return false;

	outFriends.resize(0);
	outFriends.reserve(friendCount);
	for (int i = 0; i < friendCount; ++i)
	{
		CSteamID id = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate);
		if (onlineOnly && SteamFriends()->GetFriendPersonaState(id) != k_EPersonaStateOnline)
			continue;

		outFriends.push_back(id.ConvertToUint64());
	}

	return true;
}

bool SteamSessionManager::findFriend(const std::wstring& name, uint64_t& outFriendUserHandle) const
{
	int friendCount = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
	if (friendCount < 0)
		return false;

	WildCompare wc(name);
	for (int i = 0; i < friendCount; ++i)
	{
		CSteamID id = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate);

		const char* persona = SteamFriends()->GetFriendPersonaName(id);
		if (!persona)
			continue;

		if (wc.match(mbstows(persona)))
		{
			outFriendUserHandle = id.ConvertToUint64();
			return true;
		}
	}

	return false;
}

bool SteamSessionManager::haveP2PData() const
{
	uint32 available = 0;

	if (!SteamNetworking()->IsP2PPacketAvailable(&available))
		return false;

	return available > 0;
}

uint32_t SteamSessionManager::receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const
{
	uint32_t receivedSize = 0;
	CSteamID fromUserID;

	if (!SteamNetworking()->ReadP2PPacket(data, size, &receivedSize, &fromUserID))
		return 0;

	outFromUserHandle = fromUserID.ConvertToUint64();
	return receivedSize;
}

uint32_t SteamSessionManager::getCurrentGameCount() const
{
	return m_currentGameCount;
}

IAchievementsProvider* SteamSessionManager::getAchievements() const
{
	return m_achievements;
}

ILeaderboardsProvider* SteamSessionManager::getLeaderboards() const
{
	return m_leaderboards;
}

IMatchMakingProvider* SteamSessionManager::getMatchMaking() const
{
	return m_matchMaking;
}

ISaveDataProvider* SteamSessionManager::getSaveData() const
{
	return m_saveData;
}

IStatisticsProvider* SteamSessionManager::getStatistics() const
{
	return m_statistics;
}

IUserProvider* SteamSessionManager::getUser() const
{
	return m_user;
}

bool SteamSessionManager::waitForStats()
{
	if (!m_receivedStats)
	{
		if (!::SteamUser()->BLoggedOn())
			return false;

		log::info << L"Steam; Waiting for stats..." << Endl;

		Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
		for (int32_t i = 0; i < 100 && !m_receivedStats; ++i)
		{
			if (!update())
				return false;

			if (currentThread)
				currentThread->sleep(100);

			if (!::SteamUser()->BLoggedOn())
				break;
		}
	}
	return m_receivedStats && m_receivedStatsSucceeded;
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
	T_DEBUG(L"Steam; Receieved stats (eResult = " << getSteamError(pCallback->m_eResult) << L")");
}

void SteamSessionManager::OnOverlayActivated(GameOverlayActivated_t* pCallback)
{
	m_requireUserAttention = (pCallback->m_bActive != 0);
}

void SteamSessionManager::OnP2PSessionRequest(P2PSessionRequest_t* pP2PSessionRequest)
{
	SteamNetworking()->AcceptP2PSessionWithUser(pP2PSessionRequest->m_steamIDRemote);
}

void SteamSessionManager::OnCurrentGameCount(NumberOfCurrentPlayers_t* pParam, bool bIOFailure)
{
	if (pParam->m_bSuccess)
		m_currentGameCount = uint32_t(pParam->m_cPlayers);
}

	}
}
