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
	CCallResult< SteamLeaderboards, LeaderboardScoreUploaded_t > m_callbackLeaderboardUploaded;
	bool m_uploadedScore;
	bool m_uploadedScoreSucceeded;

	void OnLeaderboardUploaded(LeaderboardScoreUploaded_t* pCallback, bool bIOFailure);
};

	}
}

#endif	// traktor_online_SteamLeaderboards_H
