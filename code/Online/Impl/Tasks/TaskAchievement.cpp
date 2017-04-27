/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/Result.h"
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
	T_ASSERT (m_provider);
	T_ASSERT (m_result);
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
