/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Online/Impl/ITask.h"

namespace traktor::online
{

class ILeaderboardsProvider;
class ScoreArrayResult;
class UserCache;

class TaskGetScores : public ITask
{
	T_RTTI_CLASS;

public:
	explicit TaskGetScores(
		ILeaderboardsProvider* leaderboardProvider,
		UserCache* userCache,
		uint64_t handle,
		int32_t from,
		int32_t to,
		bool friends,
		ScoreArrayResult* result
	);

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< ILeaderboardsProvider > m_leaderboardProvider;
	Ref< UserCache > m_userCache;
	uint64_t m_handle;
	int32_t m_from;
	int32_t m_to;
	bool m_friends;
	Ref< ScoreArrayResult > m_result;
};

}
