#include <cell/sysmodule.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_gamecontent.h>
#include <sysutil/sysutil_sysparam.h>
#include "Core/Log/Log.h"
#include "Core/System/OS.h"
#include "Core/System/PS3/SystemCallback.h"
#include "Online/Psn/PsnAchievements.h"
#include "Online/Psn/PsnLeaderboards.h"
#include "Online/Psn/PsnSaveData.h"
#include "Online/Psn/PsnStatistics.h"
#include "Online/Psn/PsnSessionManager.h"
#include "Online/Psn/PsnLogError.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

uint8_t g_npPool[SCE_NP_MIN_POOL_SIZE];

const wchar_t* lookupLanguageCode(int32_t id)
{
	switch (id)
	{
	case SCE_NP_LANG_JAPANESE:
		break;
	case SCE_NP_LANG_ENGLISH:
		return L"en";
	case SCE_NP_LANG_FRENCH:
		return L"fr";
	case SCE_NP_LANG_SPANISH:
		return L"es";
	case SCE_NP_LANG_GERMAN:
		return L"de";
	case SCE_NP_LANG_ITALIAN:
		return L"it";
	case SCE_NP_LANG_DUTCH:
		return L"nl";
	case SCE_NP_LANG_PORTUGUESE:
		return L"pt";
	case SCE_NP_LANG_RUSSIAN:
		return L"ru";
	case SCE_NP_LANG_KOREAN:
	case SCE_NP_LANG_CHINESE_T:
	case SCE_NP_LANG_CHINESE_S:
		break;
	case SCE_NP_LANG_FINNISH:
		return L"fi";
	case SCE_NP_LANG_SWEDISH:
		return L"sv";
	case SCE_NP_LANG_DANISH:
		return L"dk";
	case SCE_NP_LANG_NORWEGIAN:
		return L"no";
	case SCE_NP_LANG_POLISH:
		return L"pl";
	}
	return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.PsnSessionManager", PsnSessionManager, ISessionManagerProvider)

bool PsnSessionManager::ms_requireUserAttention = false;

PsnSessionManager::PsnSessionManager()
:	m_connected(false)
{
}

bool PsnSessionManager::create(const IGameConfiguration* configuration)
{
	// FIXME
	PsnCreateDesc desc = { 0 };

	uint64_t reqTrophySpaceBytes = 0;
	int32_t err;

	err = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP2);
	if (err < 0)
	{
		log::error << L"Unable to create session manager; cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP2) failed" << Endl;
		return false;
	}

	err = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY);
	if (err < 0)
	{
		log::error << L"Unable to create session manager; cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY) failed" << Endl;
		return false;
	}

	err = sceNp2Init(SCE_NP_MIN_POOL_SIZE, g_npPool);
	if (err < 0)
	{
		log::error << L"Unable to create session manager; sceNp2Init failed" << Endl;
		return false;
	}

	err = sceNpTrophyInit(NULL, 0, SYS_MEMORY_CONTAINER_ID_INVALID, 0);
	if (err < 0)
	{
		log::error << L"Unable to create session manager; sceNpTrophyInit failed" << Endl;
		return false;
	}

	err = sceNpTrophyCreateContext(&m_trophyContext, desc.communicationId, desc.signature, 0);
	if (err < 0) {
		log::error << L"Unable to create session manager;  sceNpTrophyCreateContext failed" << Endl;
		return false;
	}

	err = sceNpTrophyCreateHandle(&m_trophyHandle);
	if (err < 0) {
		log::error << L"Unable to create session manager; sceNpTrophyCreateHandle failed" << Endl;
		return false;
	}

	err = sceNpTrophyGetRequiredDiskSpace(m_trophyContext, m_trophyHandle, &reqTrophySpaceBytes, 0);
	if (err < 0)
	{
		log::error << PsnLogError::getTrophyErrorString(err) << Endl;
		return false;
	}

	int32_t trophySpaceKB = (reqTrophySpaceBytes + 1023) / 1024;

	m_achievements = new PsnAchievements(desc.achievements, m_trophyContext, m_trophyHandle);
	m_leaderboards = new PsnLeaderboards();
	m_saveData = new PsnSaveData(trophySpaceKB, desc.saveIconBuffer, desc.saveIconSize);
	m_statistics = new PsnStatistics();

	SystemCallback::getInstance().add(&PsnSessionManager::systemCallback);

	return true;
}

void PsnSessionManager::destroy()
{
	m_statistics = 0;
	m_saveData = 0;
	m_leaderboards = 0;
	m_achievements = 0;

	sceNpTrophyDestroyHandle(m_trophyHandle);
	sceNpTrophyDestroyContext(m_trophyContext);
	sceNpTrophyTerm();

	cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY);
}

bool PsnSessionManager::update()
{
	int32_t state;
	if (sceNpManagerGetStatus(&state) == 0)
	{
		if (state == SCE_NP_MANAGER_STATUS_ONLINE)
			m_connected = true;
		else
			m_connected = false;
	}
	else
		m_connected = false;

	return true;
}

std::wstring PsnSessionManager::getLanguageCode() const
{
#if defined(PREFER_NP_LANGUAGE)
	// Try first from user profile; requires the user to be online.
	// \note Apparently this fail TRC so it should not be used.
	SceNpMyLanguages languages;
	if (sceNpManagerGetMyLanguages(&languages) >= 0)
	{
		const wchar_t* languageCode = lookupLanguageCode(languages.language1);
		if (languageCode)
			return languageCode;

		languageCode = lookupLanguageCode(languages.language2);
		if (languageCode)
			return languageCode;

		languageCode = lookupLanguageCode(languages.language3);
		if (languageCode)
			return languageCode;
	}
#endif

	// Use system language.
	int32_t languageId;
	if (cellSysutilGetSystemParamInt(CELL_SYSUTIL_SYSTEMPARAM_ID_LANG, &languageId) == CELL_OK)
	{
		const wchar_t* languageCode = lookupLanguageCode(languageId);
		if (languageCode)
			return languageCode;
	}

	return L"";
}

bool PsnSessionManager::isConnected() const
{
	return m_connected;
}

bool PsnSessionManager::requireFullScreen() const
{
	return false;
}

bool PsnSessionManager::requireUserAttention() const
{
	return ms_requireUserAttention;
}

uint64_t PsnSessionManager::getCurrentUserHandle() const
{
	return 0;
}

bool PsnSessionManager::haveP2PData() const
{
	return false;
}

uint32_t PsnSessionManager::receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const
{
	return 0;
}

IAchievementsProvider* PsnSessionManager::getAchievements() const
{
	return m_achievements;
}

ILeaderboardsProvider* PsnSessionManager::getLeaderboards() const
{
	return m_leaderboards;
}

IMatchMakingProvider* PsnSessionManager::getMatchMaking() const
{
	return 0;
}

ISaveDataProvider* PsnSessionManager::getSaveData() const
{
	return m_saveData;
}

IStatisticsProvider* PsnSessionManager::getStatistics() const
{
	return m_statistics;
}

IUserProvider* PsnSessionManager::getUser() const
{
	return 0;
}

IVideoSharingProvider* PsnSessionManager::getVideoSharing() const
{
	return 0;
}

IVoiceChatProvider* PsnSessionManager::getVoiceChat() const
{
	return 0;
}

void PsnSessionManager::systemCallback(uint64_t status, uint64_t param)
{
	if (status == CELL_SYSUTIL_SYSTEM_MENU_OPEN)
		ms_requireUserAttention = true;
	else if (status == CELL_SYSUTIL_SYSTEM_MENU_CLOSE)
		ms_requireUserAttention = false;
}

	}
}
