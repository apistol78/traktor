#ifndef traktor_online_LeaderboardSteam_H
#define traktor_online_LeaderboardSteam_H

#include <steam/steam_api.h>
#include "Online/ILeaderboard.h"

namespace traktor
{
	namespace online
	{

class LeaderboardSteam : public ILeaderboard
{
	T_RTTI_CLASS;

public:
	LeaderboardSteam(SteamLeaderboard_t handle, uint32_t rank, int32_t score);

	virtual uint32_t getRank() const;

	virtual bool setScore(int32_t score);

	virtual int32_t getScore() const;

private:
	SteamLeaderboard_t m_handle;
	uint32_t m_rank;
	int32_t m_score;
	bool m_uploadingScore;
	CCallResult< LeaderboardSteam, LeaderboardScoreUploaded_t > m_callbackLeaderboardUploaded;

	bool synchronize() const;

	void OnLeaderboardUploaded(LeaderboardScoreUploaded_t* pCallback, bool bIOFailure);
};

	}
}

#endif	// traktor_online_LeaderboardSteam_H
