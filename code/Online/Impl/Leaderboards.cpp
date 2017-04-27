/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Online/Impl/Leaderboards.h"
#include "Online/Impl/TaskQueue.h"
#include "Online/Impl/Tasks/TaskEnumLeaderboards.h"
#include "Online/Impl/Tasks/TaskGetScores.h"
#include "Online/Impl/Tasks/TaskLeaderboard.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.Leaderboards", Leaderboards, ILeaderboards)

bool Leaderboards::ready() const
{
	return m_ready;
}

bool Leaderboards::enumerate(std::set< std::wstring >& outLeaderboardIds) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::map< std::wstring, ILeaderboardsProvider::LeaderboardData >::const_iterator i = m_leaderboards.begin(); i != m_leaderboards.end(); ++i)
		outLeaderboardIds.insert(i->first);
	return true;
}

bool Leaderboards::create(const std::wstring& leaderboardId)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	ILeaderboardsProvider::LeaderboardData leaderboardData;

	if (!m_provider->create(leaderboardId, leaderboardData))
		return false;

	m_leaderboards.insert(std::make_pair(leaderboardId, leaderboardData));
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

Ref< Result > Leaderboards::addScore(const std::wstring& leaderboardId, int32_t score)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< std::wstring, ILeaderboardsProvider::LeaderboardData >::iterator i = m_leaderboards.find(leaderboardId);
	if (i == m_leaderboards.end())
	{
		log::warning << L"Leaderboard error; No such leaderboard provided, \"" << leaderboardId << L"\"" << Endl;
		return 0;
	}

	Ref< Result > result = new Result();
	if (m_taskQueue->add(new TaskLeaderboard(
		m_provider,
		i->second.handle,
		i->second.score + score,
		result
	)))
	{
		i->second.score += score;
		return result;
	}
	else
		return 0;
}

Ref< ScoreArrayResult > Leaderboards::getGlobalScores(const std::wstring& leaderboardId, int32_t from, int32_t to)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (from <= to);

	std::map< std::wstring, ILeaderboardsProvider::LeaderboardData >::iterator i = m_leaderboards.find(leaderboardId);
	if (i == m_leaderboards.end())
	{
		log::warning << L"Leaderboard error; No such leaderboard provided, \"" << leaderboardId << L"\"" << Endl;
		return 0;
	}

	Ref< ScoreArrayResult > result = new ScoreArrayResult();
	if (m_taskQueue->add(new TaskGetScores(
		m_provider,
		m_userCache,
		i->second.handle,
		from,
		to,
		false,
		result
	)))
	{
		return result;
	}
	else
		return 0;
}

Ref< ScoreArrayResult > Leaderboards::getFriendScores(const std::wstring& leaderboardId, int32_t from, int32_t to)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (from <= to);

	std::map< std::wstring, ILeaderboardsProvider::LeaderboardData >::iterator i = m_leaderboards.find(leaderboardId);
	if (i == m_leaderboards.end())
	{
		log::warning << L"Leaderboard error; No such leaderboard provided, \"" << leaderboardId << L"\"" << Endl;
		return 0;
	}

	Ref< ScoreArrayResult > result = new ScoreArrayResult();
	if (m_taskQueue->add(new TaskGetScores(
		m_provider,
		m_userCache,
		i->second.handle,
		from,
		to,
		true,
		result
	)))
	{
		return result;
	}
	else
		return 0;
}

Leaderboards::Leaderboards(ILeaderboardsProvider* provider, UserCache* userCache, TaskQueue* taskQueue)
:	m_provider(provider)
,	m_userCache(userCache)
,	m_taskQueue(taskQueue)
,	m_ready(false)
{
}

void Leaderboards::enqueueEnumeration()
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
