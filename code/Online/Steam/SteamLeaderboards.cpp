#include "Core/Misc/TString.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Online/Steam/SteamLeaderboards.h"
#include "Online/Steam/SteamSessionManager.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamLeaderboards", SteamLeaderboards, ILeaderboardsProvider)

SteamLeaderboards::SteamLeaderboards(SteamSessionManager* sessionManager, const wchar_t** leaderboardIds)
:	m_sessionManager(sessionManager)
,	m_receivedLeaderboard(false)
,	m_receivedLeaderboardHandle(0)
,	m_receivedLeaderboardScore(0)
,	m_receivedLeaderboardRank(0)
,	m_uploadedScore(false)
,	m_uploadedScoreSucceeded(false)
{
	for (const wchar_t** leaderboardId = leaderboardIds; *leaderboardId; ++leaderboardId)
		m_leaderboardIds.insert(*leaderboardId);
}

bool SteamLeaderboards::enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards)
{
	if (!m_sessionManager->waitForStats())
		return false;

	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();

	for (std::set< std::wstring >::const_iterator i = m_leaderboardIds.begin(); i != m_leaderboardIds.end(); ++i)
	{
		SteamAPICall_t call = SteamUserStats()->FindOrCreateLeaderboard(
			wstombs(*i).c_str(),
			k_ELeaderboardSortMethodDescending,
			k_ELeaderboardDisplayTypeNumeric
		);
		if (call == 0)
			return false;

		m_receivedLeaderboard = false;
		m_receivedLeaderboardHandle = 0;
		m_callbackFindLeaderboard.Set(call, this, &SteamLeaderboards::OnLeaderboardFind);

		while (!m_receivedLeaderboard)
		{
			m_sessionManager->update();
			if (currentThread)
				currentThread->sleep(100);
		}

		if (!m_receivedLeaderboardHandle)
			return false;

		LeaderboardData data;
		data.handle = m_receivedLeaderboardHandle;
		data.score = m_receivedLeaderboardScore;
		data.rank = m_receivedLeaderboardRank;
		outLeaderboards.insert(std::make_pair(*i, data));
	}

	return true;
}

bool SteamLeaderboards::set(const uint64_t handle, int32_t score)
{
	SteamAPICall_t call = SteamUserStats()->UploadLeaderboardScore(handle, k_ELeaderboardUploadScoreMethodForceUpdate, score, 0, 0);
	if (call == 0)
		return false;

	m_uploadedScore = false;
	m_callbackLeaderboardUploaded.Set(call, this, &SteamLeaderboards::OnLeaderboardUploaded);

	while (!m_uploadedScore)
		m_sessionManager->update();

	return m_uploadedScoreSucceeded;
}

void SteamLeaderboards::OnLeaderboardFind(LeaderboardFindResult_t* pCallback, bool bIOFailure)
{
	m_callbackFindLeaderboard.Cancel();

	if (pCallback->m_hSteamLeaderboard == 0)
	{
		m_receivedLeaderboard = true;
		return;
	}

	SteamAPICall_t call = SteamUserStats()->DownloadLeaderboardEntries(pCallback->m_hSteamLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, 0, 0);
	if (call == 0)
	{
		m_receivedLeaderboard = true;
		return;
	}

	m_callbackDownloadLeaderboard.Set(call, this, &SteamLeaderboards::OnLeaderboardDownloaded);
}

void SteamLeaderboards::OnLeaderboardDownloaded(LeaderboardScoresDownloaded_t* pCallback, bool bIOFailure)
{
	m_callbackDownloadLeaderboard.Cancel();

	const char* leaderboardId = SteamUserStats()->GetLeaderboardName(pCallback->m_hSteamLeaderboard);
	if (!leaderboardId)
	{
		m_receivedLeaderboard = true;
		return;
	}

	LeaderboardEntry_t leaderboardEntry;
	leaderboardEntry.m_nGlobalRank = 0;
	leaderboardEntry.m_nScore = 0;

	if (pCallback->m_cEntryCount > 0)
		SteamUserStats()->GetDownloadedLeaderboardEntry(pCallback->m_hSteamLeaderboardEntries, 0, &leaderboardEntry, 0, 0);

	m_receivedLeaderboard = true;
	m_receivedLeaderboardHandle = pCallback->m_hSteamLeaderboard;
	m_receivedLeaderboardScore = leaderboardEntry.m_nScore;
	m_receivedLeaderboardRank = leaderboardEntry.m_nGlobalRank;
}

void SteamLeaderboards::OnLeaderboardUploaded(LeaderboardScoreUploaded_t* pCallback, bool bIOFailure)
{
	m_callbackLeaderboardUploaded.Cancel();

	m_uploadedScore = true;
	m_uploadedScoreSucceeded = (pCallback->m_bSuccess != 0);
}

	}
}
