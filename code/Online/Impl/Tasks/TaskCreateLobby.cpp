/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/LobbyResult.h"
#include "Online/Impl/Lobby.h"
#include "Online/Impl/Tasks/TaskCreateLobby.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskCreateLobby", TaskCreateLobby, ITask)

TaskCreateLobby::TaskCreateLobby(
	IMatchMakingProvider* matchMakingProvider,
	UserCache* userCache,
	uint32_t maxUsers,
	LobbyAccess access,
	LobbyResult* result
)
:	m_matchMakingProvider(matchMakingProvider)
,	m_userCache(userCache)
,	m_maxUsers(maxUsers)
,	m_access(access)
,	m_result(result)
{
}

void TaskCreateLobby::execute(TaskQueue* taskQueue)
{
	T_ASSERT(m_matchMakingProvider);
	T_ASSERT(m_userCache);
	T_ASSERT(m_result);

	uint64_t lobbyHandle;
	if (m_matchMakingProvider->createLobby(m_maxUsers, m_access, lobbyHandle))
	{
		m_result->succeed(new Lobby(
			m_matchMakingProvider,
			m_userCache,
			taskQueue,
			lobbyHandle
		));
	}
	else
		m_result->fail();
}

}
