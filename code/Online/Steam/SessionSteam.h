#ifndef traktor_online_SessionSteam_H
#define traktor_online_SessionSteam_H

#include <list>
#include <map>
#include "Core/Thread/Semaphore.h"
#include "Online/ISession.h"

namespace traktor
{
	namespace online
	{

class CurrentUserSteam;
class LeaderboardSteam;

class SessionSteam : public ISession
{
	T_RTTI_CLASS;

public:
	SessionSteam(CurrentUserSteam* user, const std::set< std::wstring >& leaderboards);

	virtual void destroy();

	virtual bool isConnected() const;

	virtual Ref< IUser > getUser();

	virtual bool rewardAchievement(const std::wstring& achievementId);

	virtual bool withdrawAchievement(const std::wstring& achievementId);

	virtual bool haveAchievement(const std::wstring& achievementId);

	virtual Ref< ILeaderboard > getLeaderboard(const std::wstring& id);

	virtual bool setStatValue(const std::wstring& statId, float value);

	virtual bool getStatValue(const std::wstring& statId, float& outValue);

	virtual Ref< ISaveGame > createSaveGame(const std::wstring& name, ISerializable* attachment);

	virtual bool getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const;

	bool update();

	STEAM_CALLBACK(SessionSteam, OnUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);

	STEAM_CALLBACK(SessionSteam, OnUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);

	STEAM_CALLBACK(SessionSteam, OnAchievementStored, UserAchievementStored_t, m_callbackAchievementStored);

private:
	mutable Semaphore m_lock;
	Ref< CurrentUserSteam > m_user;
	std::list< std::wstring > m_pendingLeaderboards;
	std::map< std::wstring, Ref< LeaderboardSteam > > m_leaderboards;
	bool m_requestedStats;
	bool m_receivedStats;
	bool m_storeStats;
	bool m_storedStats;
	bool m_requestedLeaderboard;
	bool m_destroyed;
	CCallResult< SessionSteam, LeaderboardFindResult_t > m_callbackFindLeaderboard;
	CCallResult< SessionSteam, LeaderboardScoresDownloaded_t > m_callbackDownloadLeaderboard;

	void OnLeaderboardFind(LeaderboardFindResult_t* pCallback, bool bIOFailure);

	void OnLeaderboardDownloaded(LeaderboardScoresDownloaded_t* pCallback, bool bIOFailure);
};

	}
}

#endif	// traktor_online_SessionSteam_H
