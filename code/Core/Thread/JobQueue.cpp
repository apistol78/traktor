#include "Core/Thread/Acquire.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobQueue.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.JobQueue", JobQueue, Object)

JobQueue::JobQueue()
:	m_pending(0)
{
}

JobQueue::~JobQueue()
{
	destroy();
}

bool JobQueue::create(uint32_t workerThreads, Thread::Priority priority)
{
	m_workerThreads.resize(workerThreads);
	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
	{
		m_workerThreads[i] = ThreadManager::getInstance().create(
			makeFunctor< JobQueue >(
				this,
				&JobQueue::threadWorker
			),
			L"Job queue, worker thread"
		);
		if (m_workerThreads[i])
			m_workerThreads[i]->start(priority);
		else
		{
			m_workerThreads[i] = nullptr;
			return false;
		}
	}
	return true;
}

void JobQueue::destroy()
{
	stop();

	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
		ThreadManager::getInstance().destroy(m_workerThreads[i]);

	m_workerThreads.clear();
}

Ref< Job > JobQueue::add(Functor* functor)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_jobQueueLock);
	Ref< Job > job = new Job(m_jobFinishedEvent, functor);
	m_jobQueue.push_back(job);
	m_jobQueuedEvent.pulse();
	Atomic::increment(m_pending);
	return job;
}

void JobQueue::fork(const RefArray< Functor >& functors)
{
	RefArray< Job > jobs;

	// Create jobs for given functors.
	if (functors.size() > 1)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_jobQueueLock);
		jobs.resize(functors.size());
		for (uint32_t i = 1; i < functors.size(); ++i)
		{
			jobs[i] = new Job(m_jobFinishedEvent, functors[i]);
			m_jobQueue.push_back(jobs[i]);
			Atomic::increment(m_pending);
		}
		m_jobQueuedEvent.pulse((int32_t)(functors.size() - 1));
	}

	// Execute first functor on caller thread.
	(*functors[0])();

	// Wait until all jobs has finished.
	for (uint32_t i = 1; i < jobs.size(); )
	{
		if (jobs[i]->wait())
			++i;
	}
}

bool JobQueue::wait(int32_t timeout)
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();
	while (m_pending > 0)
	{
		if (!m_jobFinishedEvent.wait(timeout))
			return false;
	}
	return true;
}

void JobQueue::stop()
{
	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
		m_workerThreads[i]->stop(0);
	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
		m_workerThreads[i]->stop();
}

void JobQueue::threadWorker()
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();
	Ref< Job > job;

	while (!thread->stopped())
	{
		if (!m_jobQueuedEvent.wait(100))
			continue;

		// Pop job from queue.
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_jobQueueLock);
			T_FATAL_ASSERT(!m_jobQueue.empty());
			job = m_jobQueue.front();
			m_jobQueue.pop_front();
		}

		// Unless job has been stopped while in queue, we execute it.
		if (job->m_finished == 0)
		{
			(*job->m_functor)();
			Atomic::exchange(job->m_finished, 1);
		}

		// Decrement number of pending jobs and signal anyone waiting for jobs to finish.
		Atomic::decrement(m_pending);
		m_jobFinishedEvent.broadcast();
	}
}

}
