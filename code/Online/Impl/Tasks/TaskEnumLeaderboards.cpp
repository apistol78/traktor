/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Online/Impl/Tasks/TaskEnumLeaderboards.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskEnumLeaderboards", TaskEnumLeaderboards, ITask)

TaskEnumLeaderboards::TaskEnumLeaderboards(
	ILeaderboardsProvider* provider,
	Object* sinkObject,
	sink_method_t sinkMethod
)
:	m_provider(provider)
,	m_sinkObject(sinkObject)
,	m_sinkMethod(sinkMethod)
{
}

void TaskEnumLeaderboards::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_provider);
	T_DEBUG(L"Online; Begin enumerating leaderboards");
	std::map< std::wstring, ILeaderboardsProvider::LeaderboardData > leaderboards;
	m_provider->enumerate(leaderboards);
	(m_sinkObject->*m_sinkMethod)(leaderboards);
	T_DEBUG(L"Online; Finished enumerating leaderboards");
}

	}
}
