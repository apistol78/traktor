/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Core/Thread/Semaphore.h"
#include "Online/ILeaderboards.h"
#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace online
	{

class ILeaderboardsProvider;
class TaskQueue;
class UserCache;

class Leaderboards : public ILeaderboards
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const override final;

	virtual bool enumerate(std::set< std::wstring >& outLeaderboardIds) const override final;

	virtual bool create(const std::wstring& leaderboardId) override final;

	virtual bool getRank(const std::wstring& leaderboardId, uint32_t& outRank) const override final;

	virtual bool getScore(const std::wstring& leaderboardId, int32_t& outScore) const override final;

	virtual Ref< Result > setScore(const std::wstring& leaderboardId, int32_t score) override final;

	virtual Ref< Result > addScore(const std::wstring& leaderboardId, int32_t score) override final;

	virtual Ref< ScoreArrayResult > getGlobalScores(const std::wstring& leaderboardId, int32_t from, int32_t to) override final;

	virtual Ref< ScoreArrayResult > getFriendScores(const std::wstring& leaderboardId, int32_t from, int32_t to) override final;

private:
	friend class SessionManager;

	Ref< ILeaderboardsProvider > m_provider;
	Ref< UserCache > m_userCache;
	Ref< TaskQueue > m_taskQueue;
	mutable Semaphore m_lock;
	std::map< std::wstring, ILeaderboardsProvider::LeaderboardData > m_leaderboards;
	bool m_ready;

	Leaderboards(ILeaderboardsProvider* provider, UserCache* userCache, TaskQueue* taskQueue);

	void enqueueEnumeration();

	void callbackEnumLeaderboards(const std::map< std::wstring, ILeaderboardsProvider::LeaderboardData >& leaderboards);
};

	}
}

