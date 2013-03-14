#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/ThreadPool.h"

namespace traktor
{
	namespace
	{

void threadPoolDispatcher(
	Event& eventAttachWork,
	Event& eventFinishedWork,
	Ref< Functor >& functorWork,
	int32_t& alive,
	int32_t& busy
)
{
	while (alive)
	{
		T_ASSERT (functorWork);
		(*functorWork)(); functorWork = 0;

		Atomic::exchange(busy, 0);
		eventFinishedWork.broadcast();

		while (alive && !eventAttachWork.wait(100))
			;
	}
}

	}

ThreadPool& ThreadPool::getInstance()
{
	static ThreadPool* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new ThreadPool();
		SingletonManager::getInstance().addBefore(s_instance, &ThreadManager::getInstance());
	}
	return *s_instance;
}

Thread* ThreadPool::spawn(Functor* functor)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	for (uint32_t i = 0; i < m_workerThreads.size(); ++i)
	{
		Worker& worker = m_workerThreads[i];
		if (Atomic::compareAndSwap(worker.busy, 0, 1) == 1)
		{
			T_ASSERT (worker.threadWorker);
			T_ASSERT (!worker.threadWorker->finished());
			T_ASSERT (!worker.functorWork);

			worker.busy = 2;
			worker.functorWork = functor;
			worker.eventFinishedWork.reset();
			worker.eventAttachWork.broadcast();

			return worker.threadWorker;
		}
	}

	if (m_workerThreads.size() >= m_workerThreads.capacity())
		return 0;

	Worker& worker = m_workerThreads.push_back();
	worker.busy = 2;
	worker.functorWork = functor;
	worker.threadWorker = ThreadManager::getInstance().create(
		makeStaticFunctor< Event&, Event&, Ref< Functor >&, int32_t&, int32_t& >(
			&threadPoolDispatcher,
			worker.eventAttachWork,
			worker.eventFinishedWork,
			worker.functorWork,
			worker.alive,
			worker.busy
		),
		L"Thread pool worker"
	);
	if (!worker.threadWorker || !worker.threadWorker->start())
	{
		m_workerThreads.pop_back();
		return 0;
	}

	return worker.threadWorker;
}

bool ThreadPool::join(Thread* thread)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (uint32_t i = 0; i < m_workerThreads.size(); ++i)
	{
		Worker& worker = m_workerThreads[i];
		if (worker.threadWorker == thread)
		{
			if (worker.busy != 0)
			{
				worker.threadWorker->stop(0);
				worker.eventFinishedWork.wait();
				worker.threadWorker->resume();
			}
			return true;
		}
	}
	T_FATAL_ERROR;
	return false;
}

void ThreadPool::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (uint32_t i = 0; i < m_workerThreads.size(); ++i)
	{
		Worker& worker = m_workerThreads[i];
		worker.threadWorker->stop();
		worker.alive = 0;
		ThreadManager::getInstance().destroy(worker.threadWorker);
	}
	m_workerThreads.clear();
}

}
