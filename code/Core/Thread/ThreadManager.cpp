/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/System.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{

ThreadManager::ThreadManager()
:	m_threadBase(new Thread(nullptr, L"Main", -1))
{
}

ThreadManager::~ThreadManager()
{
	T_EXCEPTION_GUARD_BEGIN

	delete m_threadBase;
	m_threadBase = nullptr;

	T_EXCEPTION_GUARD_END
}

ThreadManager& ThreadManager::getInstance()
{
	static ThreadManager* s_instance = nullptr;
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

	Thread* current = nullptr;

#if defined(_WIN32)
	const uint32_t currentId = GetCurrentThreadId();
	for (auto thread : m_threads)
	{
		if (thread->id() == currentId)
		{
			current = thread;
			break;
		}
	}
#else
	for (auto thread : m_threads)
	{
		if (thread->current())
		{
			current = thread;
			break;
		}
	}
#endif

	if (!current)
		current = m_threadBase;

	return current;
}

void ThreadManager::destroy()
{
	delete this;
}

Thread* ThreadManager::create(const threadFn_t& fn, const wchar_t* const name, int hardwareCore)
{
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_threadsLock);
	Thread* thread = new Thread(fn, name, hardwareCore);
	m_threads.push_back(thread);
	return thread;
}

void ThreadManager::destroy(Thread* thread)
{
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_threadsLock);
	auto i = std::find(m_threads.begin(), m_threads.end(), thread);
	if (i != m_threads.end())
	{
		m_threads.erase(i);
		delete thread;
	}
}

}
