#include "Online/Impl/Tasks/TaskUpdateSessionManager.h"
#include "Online/Provider/ISessionManagerProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskUpdateSessionManager", TaskUpdateSessionManager, ITask)

TaskUpdateSessionManager::TaskUpdateSessionManager(ISessionManagerProvider* provider)
:	m_provider(provider)
,	m_completed(false)
{
}

void TaskUpdateSessionManager::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_provider);
	m_provider->update();
	m_completed = true;
}

	}
}
