#if defined(_WIN32)
#	include <windows.h>
#endif
#include <steam/steam_api.h>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Online/Steam/CurrentUserSteam.h"
#include "Online/Steam/SessionManagerSteam.h"
#include "Online/Steam/SessionSteam.h"

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
	{ L"swedish", L"se" }
};

#if defined(_WIN32)

void steamMiniDumpTranslator(unsigned int exceptionCode, struct _EXCEPTION_POINTERS* exception)
{
	SteamAPI_SetMiniDumpComment("Traktor SteamWorks rev.1");
	SteamAPI_WriteMiniDump(exceptionCode, exception, 0);
}

#endif

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.SessionManagerSteam", 0, SessionManagerSteam, ISessionManager)

bool SessionManagerSteam::create()
{
	if (!SteamAPI_Init())
	{
		log::error << L"Unable to initialize Steam API" << Endl;
		return false;
	}

#if defined(_WIN32)
	// Hook Steam mini dump facility into SEH.
	if (!IsDebuggerPresent())
		_set_se_translator(steamMiniDumpTranslator);
#endif

	m_currentUser = new CurrentUserSteam();
	return true;
}

void SessionManagerSteam::destroy()
{
	safeDestroy(m_session);

	m_currentUser = 0;

	SteamAPI_Shutdown();
}

std::wstring SessionManagerSteam::getLanguageCode() const
{
	const char* language = SteamApps()->GetCurrentGameLanguage();
	if (!language)
		return L"";

	for (uint32_t i = 0; i < sizeof_array(c_languageCodes); ++i)
	{
		if (compareIgnoreCase(mbstows(language), c_languageCodes[i].steam) == 0)
			return c_languageCodes[i].code;
	}

	log::error << L"Session steam; unable to map language \"" << mbstows(language) << L"\" to ISO 639-1 code" << Endl;
	return L"";
}

bool SessionManagerSteam::getAvailableUsers(RefArray< IUser >& outUsers)
{
	return false;
}

Ref< IUser > SessionManagerSteam::getCurrentUser()
{
	return m_currentUser;
}

Ref< ISession > SessionManagerSteam::createSession(IUser* user, const std::set< std::wstring >& leaderboards)
{
	T_ASSERT (user == m_currentUser);
	T_ASSERT (m_session == 0);

	m_session = new SessionSteam(m_currentUser, leaderboards);
	return m_session;
}

bool SessionManagerSteam::requireUserAttention() const
{
	return SteamUtils()->IsOverlayEnabled();
}

bool SessionManagerSteam::update()
{
	if (m_session)
	{
		if (!m_session->update())
			m_session = 0;
	}

	SteamAPI_RunCallbacks();

	return true;
}

	}
}
