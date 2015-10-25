#ifndef traktor_online_SteamSessionManager_H
#define traktor_online_SteamSessionManager_H

#include <map>
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
class SteamMatchMaking;
class SteamSaveData;
class SteamStatistics;
class SteamUser;
class SteamVoiceChat;

class T_DLLCLASS SteamSessionManager : public ISessionManagerProvider
{
	T_RTTI_CLASS;

public:
	SteamSessionManager();

	virtual bool create(const IGameConfiguration* configuration) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool update() T_OVERRIDE T_FINAL;

	virtual std::wstring getLanguageCode() const T_OVERRIDE T_FINAL;

	virtual bool isConnected() const T_OVERRIDE T_FINAL;

	virtual bool requireFullScreen() const T_OVERRIDE T_FINAL;

	virtual bool requireUserAttention() const T_OVERRIDE T_FINAL;

	virtual bool haveDLC(const std::wstring& id) const T_OVERRIDE T_FINAL;

	virtual bool buyDLC(const std::wstring& id) const T_OVERRIDE T_FINAL;

	virtual bool navigateUrl(const net::Url& url) const T_OVERRIDE T_FINAL;

	virtual uint64_t getCurrentUserHandle() const T_OVERRIDE T_FINAL;

	virtual bool getFriends(std::vector< uint64_t >& outFriends, bool onlineOnly) const T_OVERRIDE T_FINAL;

	virtual bool findFriend(const std::wstring& name, uint64_t& outFriendUserHandle) const T_OVERRIDE T_FINAL;

	virtual bool haveP2PData() const T_OVERRIDE T_FINAL;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const T_OVERRIDE T_FINAL;

	virtual uint32_t getCurrentGameCount() const T_OVERRIDE T_FINAL;

	virtual IAchievementsProvider* getAchievements() const T_OVERRIDE T_FINAL;

	virtual ILeaderboardsProvider* getLeaderboards() const T_OVERRIDE T_FINAL;

	virtual IMatchMakingProvider* getMatchMaking() const T_OVERRIDE T_FINAL;

	virtual ISaveDataProvider* getSaveData() const T_OVERRIDE T_FINAL;

	virtual IStatisticsProvider* getStatistics() const T_OVERRIDE T_FINAL;

	virtual IUserProvider* getUser() const T_OVERRIDE T_FINAL;

	virtual IVideoSharingProvider* getVideoSharing() const T_OVERRIDE T_FINAL;

	virtual IVoiceChatProvider* getVoiceChat() const T_OVERRIDE T_FINAL;

	bool waitForStats();

	bool storeStats();

private:
	Ref< SteamAchievements > m_achievements;
	Ref< SteamLeaderboards > m_leaderboards;
	Ref< SteamMatchMaking > m_matchMaking;
	Ref< ISaveDataProvider > m_saveData;
	Ref< SteamStatistics > m_statistics;
	Ref< SteamUser > m_user;
	Ref< SteamVoiceChat > m_voiceChat;
	std::map< std::wstring, uint32_t > m_dlcIds;
	bool m_requireUserAttention;
	bool m_requestedStats;
	bool m_receivedStats;
	bool m_receivedStatsSucceeded;
	uint32_t m_maxRequestAttempts;
	uint32_t m_requestAttempts;
	uint32_t m_updateGameCountTicks;
	uint32_t m_currentGameCount;
	CCallResult< SteamSessionManager, NumberOfCurrentPlayers_t > m_callbackGameCount;

	STEAM_CALLBACK(SteamSessionManager, OnUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);

	STEAM_CALLBACK(SteamSessionManager, OnOverlayActivated, GameOverlayActivated_t, m_callbackOverlay);

	STEAM_CALLBACK(SteamSessionManager, OnP2PSessionRequest, P2PSessionRequest_t, m_callbackSessionRequest);

	void OnCurrentGameCount(NumberOfCurrentPlayers_t* pParam, bool bIOFailure);
};

	}
}

#endif	// traktor_online_SteamSessionManager_H
