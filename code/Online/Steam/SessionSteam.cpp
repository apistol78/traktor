#include <steam/steam_api.h>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Online/Steam/CurrentUserSteam.h"
#include "Online/Steam/SaveGameSteam.h"
#include "Online/Steam/SessionSteam.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SessionSteam", SessionSteam, ISession)

SessionSteam::SessionSteam(CurrentUserSteam* user)
:	m_callbackUserStatsReceived(this, &SessionSteam::OnUserStatsReceived)
,	m_callbackUserStatsStored(this, &SessionSteam::OnUserStatsStored)
,	m_callbackAchievementStored(this, &SessionSteam::OnAchievementStored)
,	m_user(user)
,	m_requestedStats(false)
,	m_receivedStats(false)
,	m_storeStats(false)
,	m_storedStats(false)
,	m_destroyed(false)
{
}

void SessionSteam::destroy()
{
	if (!m_destroyed)
	{
		m_destroyed = true;

		// Ensure pending stats are sent to Steam.
		if (m_storeStats)
		{
			if (SteamUser()->BLoggedOn())
			{
				m_storedStats = false;
				if (SteamUserStats()->StoreStats())
				{
					// Wait until we've received an acknowledgment; bail if more than one second.
					for (int i = 0; i < 100; ++i)
					{
						SteamAPI_RunCallbacks();
						if (m_storedStats)
							break;
						ThreadManager::getInstance().getCurrentThread()->sleep(10);
					}
					if (!m_storedStats)
						log::warning << L"Steam stats not stored properly" << Endl;
				}
			}
		}
	}
}

bool SessionSteam::isConnected() const
{
	return SteamUser()->BLoggedOn();
}

Ref< IUser > SessionSteam::getUser()
{
	return m_user;
}

bool SessionSteam::rewardAchievement(const std::wstring& achievementId)
{
	if (!m_receivedStats)
	{
		log::error << L"Unable to reward achievement \"" << achievementId << L"\"; no stats received yet" << Endl;
		return false;
	}

	if (!SteamUserStats()->SetAchievement(wstombs(achievementId).c_str()))
	{
		log::error << L"Unable to reward achievement \"" << achievementId << L"\"; SetAchievement failed" << Endl;
		return false;
	}

	m_storeStats = true;
	return true;
}

bool SessionSteam::withdrawAchievement(const std::wstring& achievementId)
{
	if (!m_receivedStats)
	{
		log::error << L"Unable to withdraw achievement \"" << achievementId << L"\"; no stats received yet" << Endl;
		return false;
	}

	if (!SteamUserStats()->ClearAchievement(wstombs(achievementId).c_str()))
	{
		log::error << L"Unable to withdraw achievement \"" << achievementId << L"\"; ClearAchievement failed" << Endl;
		return false;
	}

	m_storeStats = true;
	return true;
}

bool SessionSteam::setStatValue(const std::wstring& statId, float value)
{
	if (!SteamUserStats()->SetStat(wstombs(statId).c_str(), value))
		return false;

	m_storeStats = true;
	return true;
}

bool SessionSteam::getStatValue(const std::wstring& statId, float& outValue)
{
	return SteamUserStats()->GetStat(wstombs(statId).c_str(), &outValue);
}

Ref< ISaveGame > SessionSteam::createSaveGame(const std::wstring& name, ISerializable* attachment)
{
	DynamicMemoryStream dms(false, true);
	BinarySerializer(&dms).writeObject(attachment);

	const std::vector< uint8_t >& buffer = dms.getBuffer();
	if (!SteamRemoteStorage()->FileWrite(wstombs(name).c_str(), &buffer[0], buffer.size()))
	{
		log::error << L"Unable to create save game; Steam FileWrite failed" << Endl;
		return 0;
	}

	return new SaveGameSteam(name);
}

bool SessionSteam::getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const
{
	int32_t fileCount = SteamRemoteStorage()->GetFileCount();
	int32_t fileSize;

	for (int32_t i = 0; i < fileCount; ++i)
	{
		const char* fileName = SteamRemoteStorage()->GetFileNameAndSize(i, &fileSize);
		if (!fileName)
			continue;

		outSaveGames.push_back(new SaveGameSteam(mbstows(fileName)));
	}

	return true;
}

bool SessionSteam::update()
{
	if (!m_requestedStats)
	{
		if (SteamUser()->BLoggedOn())
		{
			if (SteamUserStats()->RequestCurrentStats())
				m_requestedStats = true;
		}
	}

	if (m_storeStats)
	{
		m_storedStats = false;
		if (SteamUser()->BLoggedOn())
		{
			if (SteamUserStats()->StoreStats())
				m_storeStats = false;
		}
	}

	return !m_destroyed;
}

void SessionSteam::OnUserStatsReceived(UserStatsReceived_t* pCallback)
{
	if (pCallback->m_eResult == k_EResultOK)
	//{
		log::debug << L"Steam session event; user stats received" << Endl;
		m_receivedStats = true;
	//}
	//else
		//log::error << L"Steam session event; failure receiving stats (m_eResult = " << int32_t(pCallback->m_eResult) << L")" << Endl;
}

void SessionSteam::OnUserStatsStored(UserStatsStored_t* pCallback)
{
	if (pCallback->m_eResult == k_EResultOK)
	{
		log::debug << L"Steam session event; user stats stored" << Endl;
		m_storedStats = true;
	}
	else
		log::error << L"Steam session event; failure storing stats (m_eResult = " << int32_t(pCallback->m_eResult) << L")" << Endl;
}

void SessionSteam::OnAchievementStored(UserAchievementStored_t* pCallback)
{
	log::debug << L"Steam session event; achievement stored" << Endl;
	m_storedStats = true;
}

	}
}
