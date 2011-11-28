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
	IMatchMakingProvider* provider,
	uint32_t maxUsers,
	LobbyResult* result
)
:	m_provider(provider)
,	m_maxUsers(maxUsers)
,	m_result(result)
{
}

void TaskCreateLobby::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_provider);
	T_ASSERT (m_result);

	IMatchMakingProvider::LobbyData providerLobby;
	if (m_provider->createLobby(m_maxUsers, providerLobby))
	{
		m_result->succeed(new Lobby(
			m_provider,
			taskQueue,
			providerLobby.handle
		));
	}
	else
		m_result->fail();
}

	}
}
