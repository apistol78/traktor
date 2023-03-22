/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/PartyResult.h"
#include "Online/Impl/Party.h"
#include "Online/Impl/Tasks/TaskCreateParty.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor::online
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
	T_ASSERT(m_matchMakingProvider);
	T_ASSERT(m_userCache);
	T_ASSERT(m_result);

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
