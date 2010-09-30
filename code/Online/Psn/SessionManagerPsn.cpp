#include <cell/sysmodule.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_sysparam.h>
#include <np.h>
#include "Core/Log/Log.h"
#include "Online/Psn/SessionPsn.h"
#include "Online/Psn/SessionManagerPsn.h"
#include "Online/Psn/UserPsn.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.SessionManagerPsn", 0, SessionManagerPsn, ISessionManager)

SessionManagerPsn::SessionManagerPsn()
{
}

bool SessionManagerPsn::create()
{
	SceNpId selfNpId;
	int32_t err;

	err = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP2);
	if (err < 0)
	{
		log::error << L"Unable to create session manager; cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP2) failed" << Endl;
		return false;
	}

	err = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP_TUS);
	if (err < 0)
	{
		log::error << L"Unable to create session manager; cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP_TUS) failed" << Endl;
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

	err = sceNpTusInit(1535);
	if (err < 0)
	{
		log::error << L"Unable to create session manager; sceNpTusInit failed" << Endl;
		return false;
	}

	err = sceNpTrophyInit(NULL, 0, SYS_MEMORY_CONTAINER_ID_INVALID, 0);
	if (err < 0)
	{
		log::error << L"Unable to create session manager; sceNpTrophyInit failed" << Endl;
		return false;
	}

	m_user = new UserPsn();

	return true;
}

void SessionManagerPsn::destroy()
{
	sceNpTusTerm();
	sceNpTrophyTerm();
	cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY);
}

std::wstring SessionManagerPsn::getLanguageCode() const
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

bool SessionManagerPsn::getAvailableUsers(RefArray< IUser >& outUsers)
{
	return true;
}

Ref< IUser > SessionManagerPsn::getCurrentUser()
{
	return m_user;
}

Ref< ISession > SessionManagerPsn::createSession(IUser* user, const std::set< std::wstring >& leaderboards)
{
	return new SessionPsn(checked_type_cast< UserPsn*, false >(user));
}

bool SessionManagerPsn::requireUserAttention() const
{
	return false;
}

bool SessionManagerPsn::update()
{
	return true;
}

	}
}
