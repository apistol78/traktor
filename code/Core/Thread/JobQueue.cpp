#include "Core/Misc/String.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobQueue.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.JobQueue", JobQueue, Object)

JobQueue::JobQueue()
:	m_running(0)
{
}

JobQueue::~JobQueue()
{
	destroy();
}

bool JobQueue::create(uint32_t workerThreads)
{
	m_workerThreads.resize(workerThreads);
	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
	{
		m_workerThreads[i] = ThreadManager::getInstance().create(
			makeFunctor< JobQueue >(
				this,
				&JobQueue::threadWorker
			),
			L"Job queue, worker thread " + toString(i)
		);
		if (m_workerThreads[i])
			m_workerThreads[i]->start(Thread::Normal);
		else
			return false;
	}
	return true;
}

void JobQueue::destroy()
{
	// Signal all worker threads we're stopping.
	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
		m_workerThreads[i]->stop(0);

	// Destroy worker threads.
	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
	{
		m_workerThreads[i]->wait();
		ThreadManager::getInstance().destroy(m_workerThreads[i]);
	}

	m_workerThreads.clear();
}

Ref< Job > JobQueue::add(Functor* functor)
{
	Ref< Job > job = new Job(functor, m_jobFinishedEvent);
	Atomic::increment(m_running);
	m_jobQueue.put(job);
	m_jobQueuedEvent.broadcast();
	return job;
}

void JobQueue::fork(const RefArray< Functor >& functors)
{
	RefArray< Job > jobs;
	if (functors.size() > 1)
	{
		jobs.resize(functors.size());
		for (uint32_t i = 1; i < functors.size(); ++i)
		{
			jobs[i] = new Job(functors[i], m_jobFinishedEvent);
			Atomic::increment(m_running);
			m_jobQueue.put(jobs[i]);
		}
		m_jobQueuedEvent.broadcast();
	}

	(*functors[0])();

	for (uint32_t i = 1; i < jobs.size(); ++i)
		jobs[i]->wait();
}

bool JobQueue::wait(int32_t timeout)
{
	for (;;)
	{
		if (m_jobQueue.empty() && m_running == 0)
			break;
		if (m_running != 0)
		{
			if (!m_jobFinishedEvent.wait(timeout))
				return false;
		}
	}
	return true;
}

void JobQueue::stop()
{
	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
		m_workerThreads[i]->stop(0);
}

int32_t JobQueue::getPendingCount() const
{
	return int32_t(m_jobQueue.size());
}

int32_t JobQueue::getRunningCount() const
{
	return m_running;
}

void JobQueue::threadWorker()
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();
	Ref< Job > job;

	for (;;)
	{
		while (
			!m_jobQueue.empty() &&
			!thread->stopped()
		)
		{
			if (m_jobQueue.get(job))
			{
				T_ASSERT (!job->m_finished);

				if (!job->m_stopped)
					(*job->m_functor)();
	
				job->m_finished = true;
				job->m_stopped = true;
				job->m_functor = 0;
				job = 0;

				Atomic::decrement(m_running);
				m_jobFinishedEvent.broadcast();
			}
		}

		if (thread->stopped())
			break;

		if (!m_jobQueuedEvent.wait(100))
			continue;
	}
}

}
