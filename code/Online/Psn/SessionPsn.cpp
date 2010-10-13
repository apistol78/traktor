#include <np.h>
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/TString.h"
#include "Online/Psn/SaveGameQueue.h"
#include "Online/Psn/SessionPsn.h"
#include "Online/Psn/UserPsn.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

const char c_signature[] = "NPWR01625";

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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SessionPsn", SessionPsn, ISession)

SessionPsn::SessionPsn(SaveGameQueue* saveGameQueue, UserPsn* user)
:	m_saveGameQueue(saveGameQueue)
,	m_user(user)
,	m_connected(false)
,	m_titleContextId(0)
{
}

bool SessionPsn::create()
{
	return true;
}

void SessionPsn::destroy()
{
	if (m_titleContextId > 0)
	{
		sceNpTusDestroyTitleCtx(m_titleContextId);
		m_titleContextId = 0;
	}
}

bool SessionPsn::isConnected() const
{
	return m_connected;
}

Ref< IUser > SessionPsn::getUser()
{
	return m_user;
}

bool SessionPsn::rewardAchievement(const std::wstring& achievementId)
{
	return true;
}

bool SessionPsn::withdrawAchievement(const std::wstring& achievementId)
{
	return false;
}

bool SessionPsn::haveAchievement(const std::wstring& achievementId)
{
	return false;
}

Ref< ILeaderboard > SessionPsn::getLeaderboard(const std::wstring& id)
{
	return 0;
}

bool SessionPsn::setStatValue(const std::wstring& statId, float value)
{
	return false;
}

bool SessionPsn::getStatValue(const std::wstring& statId, float& outValue)
{
	return false;
}

Ref< ISaveGame > SessionPsn::createSaveGame(const std::wstring& name, ISerializable* attachment)
{
	return m_saveGameQueue->createSaveGame(name, attachment);
}

bool SessionPsn::getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const
{
	return m_saveGameQueue->getAvailableSaveGames(outSaveGames);
}

void SessionPsn::update()
{
	int32_t state;
	if (sceNpManagerGetStatus(&state) == 0)
	{
		if (state == SCE_NP_MANAGER_STATUS_ONLINE)
		{
			if (!m_connected)
			{
				log::debug << L"NP CONNECTED" << Endl;

				SceNpId selfNpId;
				int32_t err;

				err = sceNpManagerGetNpId(&selfNpId);
				if (err < 0)
				{
					log::error << L"Unable to create session manager; sceNpManagerGetNpId failed" << Endl;
					//return false;
				}

				SceNpCommunicationId communicationId;
				std::memset(&communicationId, 0, sizeof(communicationId));
				strcpy(communicationId.data, c_signature);

				m_titleContextId = sceNpTusCreateTitleCtx(&communicationId, &c_passphrase, &selfNpId);
				if (m_titleContextId < 0)
				{
					log::error << L"Unable to create session; sceNpTusCreateTitleCtx failed" << Endl;
					//return false;
				}
			}

			m_connected = true;
		}
		else
		{
			if (m_connected)
			{
				log::debug << L"NP DISCONNECTED (1)" << Endl;
			}

			m_connected = false;
		}
	}
	else
	{
		if (m_connected)
		{
			log::debug << L"NP DISCONNECTED (2)" << Endl;
		}

		m_connected = false;
	}
}

	}
}
