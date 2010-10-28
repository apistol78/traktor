#ifndef traktor_online_SteamLeaderboards_H
#define traktor_online_SteamLeaderboards_H

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
	SteamLeaderboards(SteamSessionManager* sessionManager, const wchar_t** leaderboardIds);

	virtual bool enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards);

	virtual bool set(const uint64_t handle, int32_t score);

private:
	SteamSessionManager* m_sessionManager;
	std::set< std::wstring > m_leaderboardIds;
	CCallResult< SteamLeaderboards, LeaderboardFindResult_t > m_callbackFindLeaderboard;
	CCallResult< SteamLeaderboards, LeaderboardScoresDownloaded_t > m_callbackDownloadLeaderboard;
	CCallResult< SteamLeaderboards, LeaderboardScoreUploaded_t > m_callbackLeaderboardUploaded;
	bool m_receivedLeaderboard;
	SteamLeaderboard_t m_receivedLeaderboardHandle;
	int32_t m_receivedLeaderboardScore;
	int32_t m_receivedLeaderboardRank;
	bool m_uploadedScore;
	bool m_uploadedScoreSucceeded;

	void OnLeaderboardFind(LeaderboardFindResult_t* pCallback, bool bIOFailure);

	void OnLeaderboardDownloaded(LeaderboardScoresDownloaded_t* pCallback, bool bIOFailure);

	void OnLeaderboardUploaded(LeaderboardScoreUploaded_t* pCallback, bool bIOFailure);
};

	}
}

#endif	// traktor_online_SteamLeaderboards_H
