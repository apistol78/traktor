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

namespace traktor
{

class Result;

	namespace online
	{

class ILeaderboardsProvider;

class TaskLeaderboard : public ITask
{
	T_RTTI_CLASS;

public:
	TaskLeaderboard(
		ILeaderboardsProvider* provider,
		uint64_t handle,
		int32_t score,
		Result* result
	);

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< ILeaderboardsProvider > m_provider;
	uint64_t m_handle;
	int32_t m_score;
	Ref< Result > m_result;
};

	}
}

