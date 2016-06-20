#ifndef traktor_online_SteamLeaderboards_H
#define traktor_online_SteamLeaderboards_H

#include <list>
#include <steam/steam_api.h>
#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace online
	{

class SteamSessionManager;

class SteamLeaderboards : public ILeaderboardsProvider
{
	T_RTTI_CLASS;

public:
	SteamLeaderboards(SteamSessionManager* sessionManager, const std::list< std::wstring >& leaderboardIds);

	virtual bool enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards) T_OVERRIDE T_FINAL;

	virtual bool create(const std::wstring& leaderboardId, LeaderboardData& outLeaderboard) T_OVERRIDE T_FINAL;

	virtual bool set(uint64_t handle, int32_t score) T_OVERRIDE T_FINAL;

	virtual bool getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores) T_OVERRIDE T_FINAL;

	virtual bool getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores) T_OVERRIDE T_FINAL;

private:
	SteamSessionManager* m_sessionManager;
	std::set< std::wstring > m_leaderboardIds;
	CCallResult< SteamLeaderboards, LeaderboardScoreUploaded_t > m_callbackLeaderboardUploaded;
	CCallResult< SteamLeaderboards, LeaderboardScoresDownloaded_t > m_callbackLeaderboardDownloaded;
	bool m_uploadedScore;
	bool m_uploadedScoreSucceeded;
	bool m_downloadedScore;
	bool m_downloadedScoreSucceeded;
	std::vector< std::pair< uint64_t, int32_t > >* m_outScores;

	void OnLeaderboardUploaded(LeaderboardScoreUploaded_t* pCallback, bool bIOFailure);

	void OnLeaderboardDownloaded(LeaderboardScoresDownloaded_t* pCallback, bool bIOFailure);
};

	}
}

#endif	// traktor_online_SteamLeaderboards_H
