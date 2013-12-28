#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/System/OS.h"
#include "Core/Singleton/SingletonManager.h"

namespace traktor
{

JobManager& JobManager::getInstance()
{
	static JobManager* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new JobManager();
		SingletonManager::getInstance().addBefore(s_instance, &ThreadManager::getInstance());

		s_instance->m_queue = new JobQueue();
		s_instance->m_queue->create(
			OS::getInstance().getCPUCoreCount(),
			Thread::Normal
		);
	}
	return *s_instance;
}

void JobManager::destroy()
{
	safeDestroy(m_queue);
	delete this;
}

}
