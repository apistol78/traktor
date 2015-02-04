#include "Online/Result.h"
#include "Online/Impl/Tasks/TaskSetLobbyMetaValue.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskSetLobbyMetaValue", TaskSetLobbyMetaValue, ITask)

TaskSetLobbyMetaValue::TaskSetLobbyMetaValue(
	IMatchMakingProvider* provider,
	uint64_t lobbyHandle,
	const std::wstring& key,
	const std::wstring& value,
	Result* result
)
:	m_provider(provider)
,	m_lobbyHandle(lobbyHandle)
,	m_key(key)
,	m_value(value)
,	m_result(result)
{
}

void TaskSetLobbyMetaValue::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_provider);
	T_ASSERT (m_result);

	if (m_provider->setLobbyMetaValue(m_lobbyHandle, m_key, m_value))
		m_result->succeed();
	else
		m_result->fail();
}

	}
}
