#ifndef traktor_online_SteamSessionManager_H
#define traktor_online_SteamSessionManager_H

#include <steam/steam_api.h>
#include "Core/Thread/Semaphore.h"
#include "Online/Provider/ISessionManagerProvider.h"
#include "Online/Steam/SteamTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_STEAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class SteamAchievements;
class SteamLeaderboards;
class SteamSaveData;
class SteamStatistics;

class T_DLLCLASS SteamSessionManager : public ISessionManagerProvider
{
	T_RTTI_CLASS;

public:
	SteamSessionManager();

	bool create(const SteamCreateDesc& desc);

	virtual void destroy();

	virtual bool update();

	virtual std::wstring getLanguageCode() const;

	virtual bool isConnected() const;

	virtual bool requireUserAttention() const;

	virtual Ref< IAchievementsProvider > getAchievements() const;

	virtual Ref< ILeaderboardsProvider > getLeaderboards() const;

	virtual Ref< ISaveDataProvider > getSaveData() const;

	virtual Ref< IStatisticsProvider > getStatistics() const;

	bool waitForStats();

	bool storeStats();

private:
	mutable Semaphore m_lock;
	Ref< SteamAchievements > m_achievements;
	Ref< SteamLeaderboards > m_leaderboards;
	Ref< SteamSaveData > m_saveData;
	Ref< SteamStatistics > m_statistics;
	bool m_requireUserAttention;
	bool m_requestedStats;
	bool m_receivedStats;
	bool m_receivedStatsSucceeded;
	uint32_t m_maxRequestAttempts;
	uint32_t m_requestAttempts;

	STEAM_CALLBACK(SteamSessionManager, OnUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);

	STEAM_CALLBACK(SteamSessionManager, OnOverlayActivated, GameOverlayActivated_t, m_callbackOverlay);
};

	}
}

#endif	// traktor_online_SteamSessionManager_H
