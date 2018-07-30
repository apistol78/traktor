/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
	T_ASSERT (m_provider);
	T_ASSERT (m_result);
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
