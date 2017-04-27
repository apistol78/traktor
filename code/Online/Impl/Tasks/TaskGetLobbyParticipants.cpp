/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/UserArrayResult.h"
#include "Online/Impl/User.h"
#include "Online/Impl/UserCache.h"
#include "Online/Impl/Tasks/TaskGetLobbyParticipants.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskGetLobbyParticipants", TaskGetLobbyParticipants, ITask)

TaskGetLobbyParticipants::TaskGetLobbyParticipants(
	IMatchMakingProvider* matchMakingProvider,
	UserCache* userCache,
	uint64_t lobbyHandle,
	UserArrayResult* result
)
:	m_matchMakingProvider(matchMakingProvider)
,	m_userCache(userCache)
,	m_lobbyHandle(lobbyHandle)
,	m_result(result)
{
}

void TaskGetLobbyParticipants::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_matchMakingProvider);
	T_ASSERT (m_userCache);
	T_ASSERT (m_result);

	std::vector< uint64_t > userHandles;
	if (m_matchMakingProvider->getLobbyParticipants(m_lobbyHandle, userHandles))
	{
		RefArray< IUser > users;
		users.reserve(userHandles.size());
		for (std::vector< uint64_t >::iterator i = userHandles.begin(); i != userHandles.end(); ++i)
			users.push_back(m_userCache->get(*i));
		m_result->succeed(users);
	}
	else
		m_result->fail();
}

	}
}
