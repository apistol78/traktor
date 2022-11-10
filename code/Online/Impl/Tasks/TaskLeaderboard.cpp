/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Result.h"
#include "Online/Impl/Tasks/TaskLeaderboard.h"
#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskLeaderboard", TaskLeaderboard, ITask)

TaskLeaderboard::TaskLeaderboard(
	ILeaderboardsProvider* provider,
	uint64_t handle,
	int32_t score,
	Result* result
)
:	m_provider(provider)
,	m_handle(handle)
,	m_score(score)
,	m_result(result)
{
}

void TaskLeaderboard::execute(TaskQueue* taskQueue)
{
	T_ASSERT(m_provider);
	T_ASSERT(m_result);
	if (m_provider->set(
		m_handle,
		m_score
	))
		m_result->succeed();
	else
		m_result->fail();
}

	}
}
