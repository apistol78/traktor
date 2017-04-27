/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/Result.h"
#include "Online/Impl/Tasks/TaskJoinLobby.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskJoinLobby", TaskJoinLobby, ITask)

TaskJoinLobby::TaskJoinLobby(
	IMatchMakingProvider* provider,
	uint64_t lobbyHandle,
	Result* result
)
:	m_provider(provider)
,	m_lobbyHandle(lobbyHandle)
,	m_result(result)
{
}

void TaskJoinLobby::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_provider);
	T_ASSERT (m_result);

	if (m_provider->joinLobby(m_lobbyHandle))
		m_result->succeed();
	else
		m_result->fail();
}

	}
}
