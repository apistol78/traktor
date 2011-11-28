#include "Online/Impl/Lobby.h"
#include "Online/Impl/TaskQueue.h"
#include "Online/Impl/Tasks/TaskJoinLobby.h"
#include "Online/Impl/Tasks/TaskSetLobbyMetaValue.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.Lobby", Lobby, ILobby)

Ref< Result > Lobby::setMetaValue(const std::wstring& key, const std::wstring& value)
{
	Ref< Result > result = new Result();
	if (m_taskQueue->add(new TaskSetLobbyMetaValue(
		m_provider,
		m_handle,
		key,
		value,
		result
	)))
		return result;
	else
		return 0;
}

bool Lobby::getMetaValue(const std::wstring& key, std::wstring& outValue) const
{
	return false;
}

Ref< Result > Lobby::join()
{
	Ref< Result > result = new Result();
	if (m_taskQueue->add(new TaskJoinLobby(
		m_provider,
		m_handle,
		result
	)))
		return result;
	else
		return 0;
}

Ref< Result > Lobby::leave()
{
	return 0;
}

Lobby::Lobby(IMatchMakingProvider* provider, TaskQueue* taskQueue, uint64_t handle)
:	m_provider(provider)
,	m_taskQueue(taskQueue)
,	m_handle(handle)
{
}

	}
}
