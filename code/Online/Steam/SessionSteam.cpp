#include <steam/steam_api.h>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/BinarySerializer.h"
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
,	m_destroyed(false)
{
}

void SessionSteam::destroy()
{
	m_destroyed = true;
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
		return false;

	if (!SteamUserStats()->SetAchievement(wstombs(achievementId).c_str()))
		return false;

	m_storeStats = true;
	return true;
}

bool SessionSteam::withdrawAchievement(const std::wstring& achievementId)
{
	if (!m_receivedStats)
		return false;

	if (!SteamUserStats()->ClearAchievement(wstombs(achievementId).c_str()))
		return false;

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
	log::debug << L"Steam session event; user stats received" << Endl;
	m_receivedStats = true;
}

void SessionSteam::OnUserStatsStored(UserStatsStored_t* pCallback)
{
	log::debug << L"Steam session event; user stats stored" << Endl;
}

void SessionSteam::OnAchievementStored(UserAchievementStored_t* pCallback)
{
	log::debug << L"Steam session event; achievement stored" << Endl;
}

	}
}
