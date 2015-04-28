#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Online/Steam/SteamLeaderboards.h"
#include "Online/Steam/SteamSessionManager.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

class FindLeaderboardCall : public Object
{
public:
	FindLeaderboardCall()
	:	m_finished(false)
	,	m_leaderboardData(0)
	{
	}

	static Ref< FindLeaderboardCall > create(const std::wstring& leaderboardId, ILeaderboardsProvider::LeaderboardData& outLeaderboardData)
	{
		SteamAPICall_t call = SteamUserStats()->FindOrCreateLeaderboard(
			wstombs(leaderboardId).c_str(),
			k_ELeaderboardSortMethodDescending,
			k_ELeaderboardDisplayTypeNumeric
		);
		if (call == 0)
			return 0;

		Ref< FindLeaderboardCall > f = new FindLeaderboardCall();
		f->m_leaderboardData = &outLeaderboardData;
		f->m_callbackFindLeaderboard.Set(call, f.ptr(), &FindLeaderboardCall::OnLeaderboardFind);

		return f;
	}

	bool finished() const { return m_finished; }

	bool successful() const { return m_leaderboardData != 0; }

private:
	bool m_finished;
	ILeaderboardsProvider::LeaderboardData* m_leaderboardData;
	CCallResult< FindLeaderboardCall, LeaderboardFindResult_t > m_callbackFindLeaderboard;
	CCallResult< FindLeaderboardCall, LeaderboardScoresDownloaded_t > m_callbackDownloadLeaderboard;

	void OnLeaderboardFind(LeaderboardFindResult_t* pCallback, bool bIOFailure)
	{
		m_callbackFindLeaderboard.Cancel();

		// Ensure we've received a valid leaderboard handle.
		if (pCallback->m_hSteamLeaderboard == 0)
		{
			m_leaderboardData = 0;
			m_finished = true;
			return;
		}

		// We've received leaderboard; continue by chaining a request of entries.
		SteamAPICall_t call = SteamUserStats()->DownloadLeaderboardEntries(pCallback->m_hSteamLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, 0, 0);
		if (call == 0)
		{
			m_leaderboardData = 0;
			m_finished = true;
			return;
		}

		m_callbackDownloadLeaderboard.Set(call, this, &FindLeaderboardCall::OnLeaderboardDownloaded);
	}

	void OnLeaderboardDownloaded(LeaderboardScoresDownloaded_t* pCallback, bool bIOFailure)
	{
		m_callbackDownloadLeaderboard.Cancel();

		const char* leaderboardId = SteamUserStats()->GetLeaderboardName(pCallback->m_hSteamLeaderboard);
		if (!leaderboardId)
		{
			m_leaderboardData = 0;
			m_finished = true;
			return;
		}

		LeaderboardEntry_t leaderboardEntry;
		leaderboardEntry.m_nGlobalRank = 0;
		leaderboardEntry.m_nScore = 0;

		if (pCallback->m_cEntryCount > 0)
			SteamUserStats()->GetDownloadedLeaderboardEntry(pCallback->m_hSteamLeaderboardEntries, 0, &leaderboardEntry, 0, 0);

		m_leaderboardData->handle = pCallback->m_hSteamLeaderboard;
		m_leaderboardData->score = leaderboardEntry.m_nScore;
		m_leaderboardData->rank = leaderboardEntry.m_nGlobalRank;

		m_finished = true;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamLeaderboards", SteamLeaderboards, ILeaderboardsProvider)

SteamLeaderboards::SteamLeaderboards(SteamSessionManager* sessionManager, const std::list< std::wstring >& leaderboardIds)
:	m_sessionManager(sessionManager)
,	m_uploadedScore(false)
,	m_uploadedScoreSucceeded(false)
,	m_downloadedScore(false)
,	m_downloadedScoreSucceeded(false)
,	m_outScores(0)
{
	m_leaderboardIds.insert(leaderboardIds.begin(), leaderboardIds.end());
}

bool SteamLeaderboards::enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards)
{
	bool haveStats = m_sessionManager->waitForStats();

	// Create empty entries for each leaderboard.
	for (std::set< std::wstring >::const_iterator i = m_leaderboardIds.begin(); i != m_leaderboardIds.end(); ++i)
	{
		LeaderboardData data;
		data.handle = 0;
		data.score = 0;
		data.rank = 0;
		outLeaderboards[*i] = data;
	}

	if (!haveStats || m_leaderboardIds.empty())
		return true;

	RefArray< FindLeaderboardCall > calls;

	// Setup a call for every registered leaderboard.
	for (std::set< std::wstring >::const_iterator i = m_leaderboardIds.begin(); i != m_leaderboardIds.end(); ++i)
	{
		Ref< FindLeaderboardCall > call = FindLeaderboardCall::create(*i, outLeaderboards[*i]);
		if (!call)
		{
			log::error << L"Unable to enumerate leaderboards; Leaderboard \"" << *i << L"\" not available" << Endl;
			continue;
		}
		calls.push_back(call);
	}

	// Wait until all calls have been completed.
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	bool allSuccessful = true;
	for (;;)
	{
		m_sessionManager->update();
		if (currentThread)
			currentThread->sleep(100);

		bool allFinished = true;
		for (RefArray< FindLeaderboardCall >::const_iterator i = calls.begin(); i != calls.end(); ++i)
		{
			if (!(*i)->finished())
			{
				allFinished = false;
				allSuccessful &= (*i)->successful();
				break;
			}
		}
		if (allFinished)
			break;
	}

	return allSuccessful;
}

bool SteamLeaderboards::set(uint64_t handle, int32_t score)
{
	if (!handle || !::SteamUser()->BLoggedOn())
		return false;

	SteamAPICall_t call = SteamUserStats()->UploadLeaderboardScore(handle, k_ELeaderboardUploadScoreMethodForceUpdate, score, 0, 0);
	if (call == 0)
		return false;

	m_uploadedScore = false;
	m_callbackLeaderboardUploaded.Set(call, this, &SteamLeaderboards::OnLeaderboardUploaded);

	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!m_uploadedScore)
	{
		m_sessionManager->update();
		if (!m_uploadedScore && currentThread)
			currentThread->wait(100);
	}

	return m_uploadedScoreSucceeded;
}

bool SteamLeaderboards::getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores)
{
	if (!handle || !::SteamUser()->BLoggedOn())
		return false;

	SteamAPICall_t call = SteamUserStats()->DownloadLeaderboardEntries(handle, k_ELeaderboardDataRequestGlobal, from + 1, to + 1);
	if (call == 0)
		return false;

	m_downloadedScore = false;
	m_outScores = &outScores;
	m_callbackLeaderboardDownloaded.Set(call, this, &SteamLeaderboards::OnLeaderboardDownloaded);

	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!m_downloadedScore)
	{
		m_sessionManager->update();
		if (!m_uploadedScore && currentThread)
			currentThread->wait(100);
	}

	return m_downloadedScoreSucceeded;
}

bool SteamLeaderboards::getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores)
{
	if (!handle || !::SteamUser()->BLoggedOn())
		return false;

	SteamAPICall_t call = SteamUserStats()->DownloadLeaderboardEntries(handle, k_ELeaderboardDataRequestFriends, from + 1, to + 1);
	if (call == 0)
		return false;

	m_downloadedScore = false;
	m_outScores = &outScores;
	m_callbackLeaderboardDownloaded.Set(call, this, &SteamLeaderboards::OnLeaderboardDownloaded);

	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!m_downloadedScore)
	{
		m_sessionManager->update();
		if (!m_uploadedScore && currentThread)
			currentThread->wait(100);
	}

	return m_downloadedScoreSucceeded;
}

void SteamLeaderboards::OnLeaderboardUploaded(LeaderboardScoreUploaded_t* pCallback, bool bIOFailure)
{
	m_callbackLeaderboardUploaded.Cancel();

	m_uploadedScore = true;
	m_uploadedScoreSucceeded = (pCallback->m_bSuccess != 0);
}

void SteamLeaderboards::OnLeaderboardDownloaded(LeaderboardScoresDownloaded_t* pCallback, bool bIOFailure)
{
	m_callbackLeaderboardDownloaded.Cancel();

	m_outScores->reserve(pCallback->m_cEntryCount);
	for (int32_t i = 0; i < pCallback->m_cEntryCount; ++i)
	{
		LeaderboardEntry_t entry;
		if (SteamUserStats()->GetDownloadedLeaderboardEntry(pCallback->m_hSteamLeaderboardEntries, i, &entry, 0, 0))
		{
			m_outScores->push_back(std::make_pair(
				entry.m_steamIDUser.ConvertToUint64(),
				entry.m_nScore
			));
		}
	}

	m_downloadedScore = true;
	m_downloadedScoreSucceeded = true;
}

	}
}
