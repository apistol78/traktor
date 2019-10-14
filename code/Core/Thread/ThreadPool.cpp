#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Singleton/SingletonManager.h"
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
	const int32_t& alive
)
{
	for (;;)
	{
		// Wait until work has been attached.
		while (alive)
		{
			if (eventAttachWork.wait(100))
				break;
		}
		if (!alive)
			break;

		// Execute work.
		(*functorWork)();
		functorWork = nullptr;

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

bool ThreadPool::spawn(Functor* functor, Thread*& outThread, Thread::Priority priority)
{
	for (uint32_t i = 0; i < sizeof_array(m_workerThreads); ++i)
	{
		Worker& worker = m_workerThreads[i];
		if (Atomic::compareAndSwap(worker.busy, 0, 1) == 0)
		{
			if (worker.thread == nullptr)
			{
				worker.thread = ThreadManager::getInstance().create(
					makeStaticFunctor< Event&, Event&, Ref< Functor >&, const int32_t& >(
						&threadPoolDispatcher,
						worker.eventAttachWork,
						worker.eventFinishedWork,
						worker.functorWork,
						worker.alive
					),
					L"Thread pool worker"
				);
				if (worker.thread)
					worker.thread->start(priority);
				else
				{
					Atomic::exchange(worker.busy, 0);
					return false;
				}
			}

			outThread = worker.thread;
			outThread->resume(priority);

			worker.functorWork = functor;
			worker.eventFinishedWork.reset();
			worker.eventAttachWork.broadcast();
			return true;
		}
	}

	// No more slots available.
	return false;
}

bool ThreadPool::join(Thread* thread)
{
	for (uint32_t i = 0; i < sizeof_array(m_workerThreads); ++i)
	{
		Worker& worker = m_workerThreads[i];
		if (worker.thread == thread)
		{
			worker.eventFinishedWork.wait();
			worker.thread->resume(Thread::Normal);
			Atomic::exchange(worker.busy, 0);
			return true;
		}
	}
	return false;
}

bool ThreadPool::stop(Thread* thread)
{
	for (uint32_t i = 0; i < sizeof_array(m_workerThreads); ++i)
	{
		Worker& worker = m_workerThreads[i];
		if (worker.thread == thread)
		{
			worker.thread->stop(0);
			worker.eventFinishedWork.wait();
			worker.thread->resume(Thread::Normal);
			Atomic::exchange(worker.busy, 0);
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
		worker.alive = 0;
		if (worker.thread != nullptr)
		{
			worker.thread->stop();
			ThreadManager::getInstance().destroy(worker.thread);
			worker.thread = nullptr;
		}
		Atomic::exchange(worker.busy, 0);
	}
}

}
