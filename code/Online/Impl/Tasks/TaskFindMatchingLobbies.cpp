/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/LobbyArrayResult.h"
#include "Online/Impl/Lobby.h"
#include "Online/Impl/Tasks/TaskFindMatchingLobbies.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskFindMatchingLobbies", TaskFindMatchingLobbies, ITask)

TaskFindMatchingLobbies::TaskFindMatchingLobbies(
	IMatchMakingProvider* matchMakingProvider,
	UserCache* userCache,
	const LobbyFilter* filter,
	LobbyArrayResult* result
)
:	m_matchMakingProvider(matchMakingProvider)
,	m_userCache(userCache)
,	m_filter(filter)
,	m_result(result)
{
}

void TaskFindMatchingLobbies::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_matchMakingProvider);
	T_ASSERT (m_userCache);
	T_ASSERT (m_result);

	std::vector< uint64_t > providerLobbies;
	if (m_matchMakingProvider->findMatchingLobbies(m_filter, providerLobbies))
	{
		RefArray< ILobby > lobbies;
		lobbies.reserve(providerLobbies.size());
		for (std::vector< uint64_t >::iterator i = providerLobbies.begin(); i != providerLobbies.end(); ++i)
		{
			lobbies.push_back(new Lobby(
				m_matchMakingProvider,
				m_userCache,
				taskQueue,
				*i
			));
		}
		m_result->succeed(lobbies);
	}
	else
		m_result->fail();
}

	}
}
