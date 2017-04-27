/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{

ThreadManager::ThreadManager()
:	m_threadBase(new Thread(0, L"Main", -1))
{
}

ThreadManager::~ThreadManager()
{
	T_EXCEPTION_GUARD_BEGIN

	delete m_threadBase, m_threadBase = 0;

	T_EXCEPTION_GUARD_END
}

ThreadManager& ThreadManager::getInstance()
{
	static ThreadManager* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new ThreadManager();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

Thread* ThreadManager::getCurrentThread()
{
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_threadsLock);

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
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_threadsLock);
	Thread* thread = new Thread(functor, name, hardwareCore);
	m_threads.push_back(thread);
	return thread;
}

void ThreadManager::destroy(Thread* thread)
{
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_threadsLock);
	std::vector< Thread* >::iterator i = std::find(m_threads.begin(), m_threads.end(), thread);
	if (i != m_threads.end())
	{
		m_threads.erase(i);
		delete thread;
	}
}

}
