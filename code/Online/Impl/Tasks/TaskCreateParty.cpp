/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/PartyResult.h"
#include "Online/Impl/Party.h"
#include "Online/Impl/Tasks/TaskCreateParty.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskCreateParty", TaskCreateParty, ITask)

TaskCreateParty::TaskCreateParty(
	IMatchMakingProvider* matchMakingProvider,
	UserCache* userCache,
	PartyResult* result
)
:	m_matchMakingProvider(matchMakingProvider)
,	m_userCache(userCache)
,	m_result(result)
{
}

void TaskCreateParty::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_matchMakingProvider);
	T_ASSERT (m_userCache);
	T_ASSERT (m_result);

	uint64_t partyHandle;
	if (m_matchMakingProvider->createParty(partyHandle))
	{
		m_result->succeed(new Party(
			m_matchMakingProvider,
			m_userCache,
			taskQueue,
			partyHandle
		));
	}
	else
		m_result->fail();
}

	}
}
