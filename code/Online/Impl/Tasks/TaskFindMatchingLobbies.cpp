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
	IMatchMakingProvider* provider,
	const std::wstring& key,
	const std::wstring& value,
	LobbyArrayResult* result
)
:	m_provider(provider)
,	m_key(key)
,	m_value(value)
,	m_result(result)
{
}

void TaskFindMatchingLobbies::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_provider);
	T_ASSERT (m_result);

	std::vector< IMatchMakingProvider::LobbyData > providerLobbies;
	if (m_provider->findMatchingLobbies(m_key, m_value, providerLobbies))
	{
		RefArray< ILobby > lobbies;
		lobbies.reserve(providerLobbies.size());
		for (std::vector< IMatchMakingProvider::LobbyData >::iterator i = providerLobbies.begin(); i != providerLobbies.end(); ++i)
		{
			lobbies.push_back(new Lobby(
				m_provider,
				taskQueue,
				i->handle
			));
		}
		m_result->succeed(lobbies);
	}
	else
		m_result->fail();
}

	}
}
