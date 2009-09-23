#include <algorithm>
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/Acquire.h"
#include "Core/Singleton/SingletonManager.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ThreadManager", ThreadManager, Singleton)

ThreadManager::ThreadManager()
:	m_threadBase(new Thread(0, L"Base", -1))
{
}

ThreadManager::~ThreadManager()
{
	T_EXCEPTION_GUARD_BEGIN

	T_ASSERT(m_threads.empty());
	delete m_threadBase;

	T_EXCEPTION_GUARD_END
}

ThreadManager& ThreadManager::getInstance()
{
	static ThreadManager* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new ThreadManager();
		SingletonManager::getInstance().addAfter(s_instance, &Heap::getInstance());
	}
	return *s_instance;
}

Thread* ThreadManager::getCurrentThread()
{
	Acquire< CriticalSection > scopeAcquire(m_threadsLock);

	Thread* current = 0;
	for (std::vector< Thread* >::iterator i = m_threads.begin(); i != m_threads.end(); ++i)
	{
		if ((*i)->current())
		{
			T_ASSERT(current == 0);
			current = *i;
		}
	}

	if (!current)
		current = m_threadBase;

	return current;
}

void ThreadManager::destroy()
{
	delete this;
}

Thread* ThreadManager::create(Functor* functor, const std::wstring& name, int hardwareCore)
{
	Acquire< CriticalSection > scopeAcquire(m_threadsLock);
	Thread* thread = new Thread(functor, name, hardwareCore);
	m_threads.push_back(thread);
	return thread;
}

void ThreadManager::destroy(Thread* thread)
{
	Acquire< CriticalSection > scopeAcquire(m_threadsLock);
	std::vector< Thread* >::iterator i = std::find(m_threads.begin(), m_threads.end(), thread);
	if (i != m_threads.end())
	{
		m_threads.erase(i);
		delete thread->m_functor;
		delete thread;
	}
}

}
