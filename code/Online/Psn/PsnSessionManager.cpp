#include <cell/sysmodule.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_gamecontent.h>
#include <sysutil/sysutil_sysparam.h>
#include "Core/Log/Log.h"
#include "Core/System/OS.h"
#include "Online/Psn/PsnAchievements.h"
#include "Online/Psn/PsnLeaderboards.h"
#include "Online/Psn/PsnSaveData.h"
#include "Online/Psn/PsnStatistics.h"
#include "Online/Psn/PsnSessionManager.h"

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
		return L"jp";
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
	case SCE_NP_LANG_PORTUGUESE:
	case SCE_NP_LANG_RUSSIAN:
	case SCE_NP_LANG_KOREAN:
	case SCE_NP_LANG_CHINESE_T:
	case SCE_NP_LANG_CHINESE_S:
		break;
	case SCE_NP_LANG_FINNISH:
		return L"fi";
	case SCE_NP_LANG_SWEDISH:
		return L"se";
	case SCE_NP_LANG_DANISH:
		return L"dk";
	case SCE_NP_LANG_NORWEGIAN:
		return L"no";
	}
	return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.PsnSessionManager", PsnSessionManager, ISessionManagerProvider)

PsnSessionManager::PsnSessionManager()
:	m_connected(false)
{
}

bool PsnSessionManager::create(const PsnCreateDesc& desc)
{
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

	m_achievements = new PsnAchievements(desc.achievements, m_trophyContext, m_trophyHandle);
	m_leaderboards = new PsnLeaderboards();
	m_saveData = new PsnSaveData();
	m_statistics = new PsnStatistics();

	return true;
}

void PsnSessionManager::destroy()
{
	int32_t err;

	m_statistics = 0;
	m_saveData = 0;
	m_leaderboards = 0;
	m_achievements = 0;

	err = sceNpTrophyDestroyHandle(m_trophyHandle);
	if (err < 0)
		log::error << L"Unable to destroy trophy handle; sceNpTrophyDestroyHandle failed" << Endl;

	err = sceNpTrophyDestroyContext(m_trophyContext);
	if (err < 0)
		log::error << L"Unable to destroy trophy context; sceNpTrophyDestroyContext failed" << Endl;

	err = sceNpTrophyTerm();
	if (err < 0)
		log::error << L"Unable to terminate trophy utility; sceNpTrophyTerm failed" << Endl;

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
	// Try first from user profile; requires the user to be online.
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

bool PsnSessionManager::requireUserAttention() const
{
	return false;
}

Ref< IAchievementsProvider > PsnSessionManager::getAchievements() const
{
	return m_achievements;
}

Ref< ILeaderboardsProvider > PsnSessionManager::getLeaderboards() const
{
	return m_leaderboards;
}

Ref< ISaveDataProvider > PsnSessionManager::getSaveData() const
{
	return m_saveData;
}

Ref< IStatisticsProvider > PsnSessionManager::getStatistics() const
{
	return m_statistics;
}

	}
}
