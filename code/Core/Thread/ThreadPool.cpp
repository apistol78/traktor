/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/ThreadPool.h"

namespace traktor
{
	namespace
	{

void threadPoolDispatcher(
	Event& eventAttachWork,
	Event& eventFinishedWork,
	const ThreadPool::threadPoolFn_t& fn,
	const std::atomic< bool >& alive
)
{
	while (alive)
	{
		// Wait until work has been attached.
		if (!eventAttachWork.wait(100))
			continue;

		// Execute work.
		T_ASSERT(fn != nullptr);
		fn();

		// Signal work has finished.
		eventFinishedWork.broadcast();
	}
}

	}

ThreadPool& ThreadPool::getInstance()
{
	static ThreadPool* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new ThreadPool();
		SingletonManager::getInstance().addBefore(s_instance, &ThreadManager::getInstance());
	}
	return *s_instance;
}

bool ThreadPool::spawn(const threadPoolFn_t& fn, Thread*& outThread, Thread::Priority priority)
{
	for (uint32_t i = 0; i < sizeof_array(m_workerThreads); ++i)
	{
		Worker& worker = m_workerThreads[i];

		// Check if worker is busy, if not we will acquire this worker.
		bool expected = false;
		if (!worker.busy.compare_exchange_strong(expected, true))
			continue;

		if (worker.thread == nullptr)
		{
			T_ASSERT(worker.alive);
			worker.thread = ThreadManager::getInstance().create(
				[&]() {
					threadPoolDispatcher(worker.eventAttachWork, worker.eventFinishedWork, worker.fn, worker.alive);
				},
				L"Thread pool worker"
			);
			if (!worker.thread)
			{
				worker.busy = false;
				continue;
			}
			worker.thread->start(priority);
		}

		outThread = worker.thread;
		outThread->resume(priority);

		worker.fn = fn;
		worker.eventFinishedWork.reset();
		worker.eventAttachWork.broadcast();
		return true;
	}

	// No more slots available.
	outThread = nullptr;
	return false;
}

bool ThreadPool::join(Thread*& thread)
{
	if (!thread)
		return true;

	for (uint32_t i = 0; i < sizeof_array(m_workerThreads); ++i)
	{
		Worker& worker = m_workerThreads[i];
		if (worker.thread == thread)
		{
			worker.eventFinishedWork.wait();
			worker.thread->resume(Thread::Normal);
			worker.busy = false;
			thread = nullptr;
			return true;
		}
	}

	return false;
}

bool ThreadPool::stop(Thread*& thread)
{
	if (!thread)
		return true;

	for (uint32_t i = 0; i < sizeof_array(m_workerThreads); ++i)
	{
		Worker& worker = m_workerThreads[i];
		if (worker.thread == thread)
		{
			worker.thread->stop(0);
			worker.eventFinishedWork.wait();
			worker.thread->resume(Thread::Normal);
			worker.busy = false;
			thread = nullptr;
			return true;
		}
	}

	return false;
}

void ThreadPool::destroy()
{
	for (uint32_t i = 0; i < sizeof_array(m_workerThreads); ++i)
	{
		Worker& worker = m_workerThreads[i];
		worker.alive = false;
		if (worker.thread != nullptr)
		{
			worker.thread->stop();
			ThreadManager::getInstance().destroy(worker.thread);
			worker.thread = nullptr;
		}
		worker.busy = false;
	}
}

}
