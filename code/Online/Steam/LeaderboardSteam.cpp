#include "Core/Log/Log.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Online/Steam/LeaderboardSteam.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LeaderboardSteam", LeaderboardSteam, ILeaderboard)

LeaderboardSteam::LeaderboardSteam(SteamLeaderboard_t handle, uint32_t rank, int32_t score)
:	m_handle(handle)
,	m_rank(rank)
,	m_score(score)
,	m_uploadingScore(false)
{
}

uint32_t LeaderboardSteam::getRank() const
{
	synchronize();
	return m_rank;
}

bool LeaderboardSteam::setScore(int32_t score)
{
	if (!synchronize())
		return false;

	SteamAPICall_t call = SteamUserStats()->UploadLeaderboardScore(m_handle, k_ELeaderboardUploadScoreMethodForceUpdate, score, 0, 0);
	if (call == 0)
		return false;

	m_callbackLeaderboardUploaded.Set(call, this, &LeaderboardSteam::OnLeaderboardUploaded);
	m_uploadingScore = true;

	return true;
}

int32_t LeaderboardSteam::getScore() const
{
	synchronize();
	return m_score;
}

bool LeaderboardSteam::synchronize() const
{
	if (!m_uploadingScore)
		return true;

	for (int32_t tries = 1000; tries >= 0; --tries)
	{
		SteamAPI_RunCallbacks();

		if (!m_uploadingScore)
			break;

		ThreadManager::getInstance().getCurrentThread()->sleep(10);
	}

	return true;
}

void LeaderboardSteam::OnLeaderboardUploaded(LeaderboardScoreUploaded_t* pCallback, bool bIOFailure)
{
	m_uploadingScore = false;

	if (!pCallback->m_bSuccess)
	{
		log::error << L"Steam session event; failed to upload score" << Endl;
		return;
	}

	log::debug << L"Steam session event; score uploaded" << Endl;

	m_score = pCallback->m_nScore;
	m_rank = pCallback->m_nGlobalRankNew;
}

	}
}
