/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Result.h"
#include "Online/Impl/Tasks/TaskAchievement.h"
#include "Online/Provider/IAchievementsProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskAchievement", TaskAchievement, ITask)

TaskAchievement::TaskAchievement(
	IAchievementsProvider* provider,
	const std::wstring& achievementId,
	bool reward,
	Result* result
)
:	m_provider(provider)
,	m_achievementId(achievementId)
,	m_reward(reward)
,	m_result(result)
{
}

void TaskAchievement::execute(TaskQueue* taskQueue)
{
	T_ASSERT(m_provider);
	T_ASSERT(m_result);
	if (m_provider->set(
		m_achievementId,
		m_reward
	))
		m_result->succeed();
	else
		m_result->fail();
}

	}
}
