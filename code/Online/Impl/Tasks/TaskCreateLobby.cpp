#include "Online/LobbyResult.h"
#include "Online/Impl/Lobby.h"
#include "Online/Impl/Tasks/TaskCreateLobby.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
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
	T_ASSERT (m_matchMakingProvider);
	T_ASSERT (m_userCache);
	T_ASSERT (m_result);

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
}
