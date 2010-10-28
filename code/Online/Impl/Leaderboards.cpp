#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Online/Impl/Leaderboards.h"
#include "Online/Impl/TaskQueue.h"
#include "Online/Impl/Tasks/TaskEnumLeaderboards.h"
#include "Online/Impl/Tasks/TaskLeaderboard.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.Leaderboards", Leaderboards, ILeaderboards)

bool Leaderboards::ready() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_ready;
}

bool Leaderboards::enumerate(std::set< std::wstring >& outLeaderboardIds) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::map< std::wstring, ILeaderboardsProvider::LeaderboardData >::const_iterator i = m_leaderboards.begin(); i != m_leaderboards.end(); ++i)
		outLeaderboardIds.insert(i->first);
	return true;
}

bool Leaderboards::getRank(const std::wstring& leaderboardId, uint32_t& outRank) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< std::wstring, ILeaderboardsProvider::LeaderboardData >::const_iterator i = m_leaderboards.find(leaderboardId);
	if (i == m_leaderboards.end())
	{
		log::warning << L"Leaderboard error; No such leaderboard provided, \"" << leaderboardId << L"\"" << Endl;
		return false;
	}

	outRank = i->second.rank;
	return true;
}

bool Leaderboards::getScore(const std::wstring& leaderboardId, int32_t& outScore) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< std::wstring, ILeaderboardsProvider::LeaderboardData >::const_iterator i = m_leaderboards.find(leaderboardId);
	if (i == m_leaderboards.end())
	{
		log::warning << L"Leaderboard error; No such leaderboard provided, \"" << leaderboardId << L"\"" << Endl;
		return false;
	}

	outScore = i->second.score;
	return true;
}

Ref< Result > Leaderboards::setScore(const std::wstring& leaderboardId, int32_t score)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< std::wstring, ILeaderboardsProvider::LeaderboardData >::iterator i = m_leaderboards.find(leaderboardId);
	if (i == m_leaderboards.end())
	{
		log::warning << L"Leaderboard error; No such leaderboard provided, \"" << leaderboardId << L"\"" << Endl;
		return 0;
	}

	if (i->second.score == score)
		return new Result(true);

	Ref< Result > result = new Result();
	if (m_taskQueue->add(new TaskLeaderboard(
		m_provider,
		i->second.handle,
		score,
		result
	)))
	{
		i->second.score = score;
		return result;
	}
	else
		return 0;
}

Leaderboards::Leaderboards(ILeaderboardsProvider* provider, TaskQueue* taskQueue)
:	m_provider(provider)
,	m_taskQueue(taskQueue)
,	m_ready(false)
{
	m_taskQueue->add(new TaskEnumLeaderboards(
		m_provider,
		this,
		(TaskEnumLeaderboards::sink_method_t)&Leaderboards::callbackEnumLeaderboards
	));
}

void Leaderboards::callbackEnumLeaderboards(const std::map< std::wstring, ILeaderboardsProvider::LeaderboardData >& leaderboards)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_leaderboards = leaderboards;
	m_ready = true;
}

	}
}
