#include <cell/sysmodule.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_gamecontent.h>
#include <sysutil/sysutil_sysparam.h>
#include <np.h>
#include "Core/Log/Log.h"
#include "Online/Psn/SaveGameQueue.h"
#include "Online/Psn/SessionPsn.h"
#include "Online/Psn/SessionManagerPsn.h"
#include "Online/Psn/UserPsn.h"
#include "Online/Psn/LogError.h"
#include "Online/Psn/CreateTrophyContextTask.h"

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

const SceNpCommunicationId c_commId = {
	{'N','P','W','R','0','1','6','2','5'},
	0,
	0,
	0
};

const SceNpCommunicationPassphrase c_passphrase =
{
	{
		0x0a,0x61,0x7e,0xdc,0x62,0x28,0x2e,0xde,
			0x6d,0x1f,0x82,0x78,0xc1,0x97,0xbc,0x57,
			0xab,0xc5,0xe5,0x18,0xf4,0xfa,0x5d,0x0b,
			0xc4,0x01,0xe4,0x95,0x33,0xca,0x9d,0xf5,
			0x1e,0x17,0xc6,0x77,0xc6,0xe8,0x51,0xd1,
			0x27,0xfe,0x4d,0xc7,0x79,0x5f,0x5a,0x33,
			0x89,0x65,0x16,0xdc,0x92,0x2c,0x95,0xfa,
			0xb9,0x56,0xb1,0x50,0xf3,0x73,0x74,0xd5,
			0x36,0xd3,0x8a,0x35,0xf4,0xbd,0xab,0xba,
			0x3e,0x88,0x2d,0x8a,0x4a,0xfe,0x25,0x4f,
			0xaa,0x4c,0x67,0x5f,0x30,0x06,0x0f,0x42,
			0xd6,0x88,0xec,0xc8,0x39,0x60,0x81,0xb2,
			0x87,0x8d,0x1f,0x99,0xd4,0xb5,0x3e,0x40,
			0xf2,0xa6,0x4f,0x46,0xc1,0x28,0xc5,0xf5,
			0x30,0x08,0xc7,0x60,0xba,0xb0,0x30,0x70,
			0xe5,0xdf,0x24,0xe7,0xde,0x9f,0xd0,0x7a
	}
};

const SceNpCommunicationSignature c_commsign = {
	{
		0xb9,0xdd,0xe1,0x3b,0x01,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0xf5,0x35,0x82,0x08,
			0x7f,0xd8,0xbb,0xba,0x85,0x7f,0x73,0xf1,
			0x08,0x94,0x53,0x25,0xaf,0xbc,0x27,0xe8,
			0x0c,0xab,0xa4,0xb9,0xf5,0x8c,0x40,0x99,
			0x12,0x12,0xba,0x16,0xb3,0x01,0xa8,0x9b,
			0xda,0xa6,0x85,0x16,0x3d,0x42,0x15,0xa3,
			0x3d,0xc1,0x0a,0x68,0x33,0xd6,0x54,0xd6,
			0xbd,0x55,0x29,0xf8,0xf1,0xfc,0x61,0x2c,
			0x59,0x96,0xad,0x8f,0x12,0xc7,0xbb,0xef,
			0x01,0x10,0x56,0x82,0x1e,0xe4,0xfe,0xe6,
			0xf0,0xed,0x03,0x1a,0xc5,0xe1,0xb1,0xd4,
			0xa7,0x54,0xc7,0xa0,0x0e,0x2d,0xe9,0x35,
			0x1b,0xcb,0x68,0x62,0x8a,0x98,0xfe,0x21,
			0x7d,0x98,0x68,0x06,0x33,0x21,0xed,0xc0,
			0xbc,0xb6,0x86,0x29,0xca,0x8b,0xe2,0x45,
			0x9e,0xfd,0x73,0x8b,0x85,0x4f,0x5e,0xda,
			0xa7,0x53,0xc5,0x2e,0xf0,0x0c,0xaf,0x70,
			0x42,0x51,0x89,0xe9,0x2d,0x86,0x93,0x61,
			0x34,0x4a,0xb8,0xae,0x05,0xd7,0x28,0x2e
	}
};


		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.SessionManagerPsn", 0, SessionManagerPsn, ISessionManager)

SessionManagerPsn::SessionManagerPsn()
:	m_trophyContext(SCE_NP_TROPHY_INVALID_CONTEXT)
,	m_trophyHandle(SCE_NP_TROPHY_INVALID_HANDLE)
{
}

bool SessionManagerPsn::create()
{
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

	err = sceNpTrophyCreateContext(&m_trophyContext, &c_commId, &c_commsign, 0);
	if (err < 0) {
		log::error << L"Unable to create session manager;  sceNpTrophyCreateContext failed" << Endl;
		return false;
	}

	err = sceNpTrophyCreateHandle(&m_trophyHandle);
	if (err < 0) {
		log::error << L"Unable to create session manager; sceNpTrophyCreateHandle failed" << Endl;
		return false;
	}

	uint64_t reqTrophySpaceBytes = 0;
	err = sceNpTrophyGetRequiredDiskSpace(m_trophyContext, m_trophyHandle, &reqTrophySpaceBytes, 0);
	if (err < 0)
		LogError::logTrophyError(err);
	int32_t trophySpaceKB = (reqTrophySpaceBytes + 1023) / 1024;

	m_saveGameQueue = new SaveGameQueue();
	if (!m_saveGameQueue->create(trophySpaceKB))
	{
		log::error << L"Unable to create session manager; failed to create save game queue" << Endl;
		return false;
	}

	if (!m_saveGameQueue->registerTrophyContext(m_trophyContext, m_trophyHandle))
	{
		log::error << L"Unable to create session manager; failed to create trophy context task" << Endl;
		return false;
	}

	m_user = new UserPsn();

	return true;
}

void SessionManagerPsn::destroy()
{
	if (m_saveGameQueue)
	{
		m_saveGameQueue->destroy();
		m_saveGameQueue = 0;
	}

	sceNpTusTerm();

	int err;
	err = sceNpTrophyDestroyHandle(m_trophyHandle);
	if ( err < 0 )
		log::error << L"Unable to destroy trophy handle. sceNpTrophyDestroyHandle failed" << Endl;
	err = sceNpTrophyDestroyContext(m_trophyContext);
	if ( err < 0 )
		log::error << L"Unable to destroy trophy context. sceNpTrophyDestroyContext failed" << Endl;
	err = sceNpTrophyTerm();
	if ( err < 0 )
		log::error << L"Unable to terminate trophy utility. sceNpTrophyTerm failed" << Endl;

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
	Ref< SessionPsn > session = new SessionPsn(m_saveGameQueue, checked_type_cast< UserPsn*, false >(user), m_trophyContext, m_trophyHandle);
	if (session->create())
	{
		m_sessions.push_back(session);
		return session;
	}
	else
		return 0;
}

bool SessionManagerPsn::requireUserAttention() const
{
	return false;
}

bool SessionManagerPsn::update()
{
	for (RefArray< SessionPsn >::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
		(*i)->update();

	return true;
}

	}
}
