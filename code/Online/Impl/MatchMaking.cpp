#include "Online/Impl/MatchMaking.h"
#include "Online/Impl/TaskQueue.h"
#include "Online/Impl/Tasks/TaskCreateLobby.h"
#include "Online/Impl/Tasks/TaskFindMatchingLobbies.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.MatchMaking", MatchMaking, IMatchMaking)

bool MatchMaking::ready() const
{
	return true;
}

Ref< LobbyArrayResult > MatchMaking::findMatchingLobbies(const std::wstring& key, const std::wstring& value)
{
	Ref< LobbyArrayResult > result = new LobbyArrayResult();
	if (m_taskQueue->add(new TaskFindMatchingLobbies(
		m_provider,
		key,
		value,
		result
	)))
		return result;
	else
		return 0;
}

Ref< LobbyResult > MatchMaking::createLobby(uint32_t maxUsers)
{
	Ref< LobbyResult > result = new LobbyResult();
	if (m_taskQueue->add(new TaskCreateLobby(
		m_provider,
		maxUsers,
		result
	)))
		return result;
	else
		return 0;
}

MatchMaking::MatchMaking(IMatchMakingProvider* provider, TaskQueue* taskQueue)
:	m_provider(provider)
,	m_taskQueue(taskQueue)
{
}

	}
}
