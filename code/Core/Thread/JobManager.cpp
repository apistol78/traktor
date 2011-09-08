#include <limits>
#include "Core/Platform.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Atomic.h"
#include "Core/System/OS.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Misc/String.h"

namespace traktor
{

bool Job::wait(int32_t timeout)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!m_finished && !currentThread->stopped())
	{
		if (!m_jobFinishedEvent.wait(timeout >= 0 ? timeout : 100))
		{
			if (timeout >= 0)
				return m_finished;
		}
		// A job has been finished; check if it this
		// and in such case return true.
	}
	return true;
}

bool Job::stopped() const
{
	if (m_stopped)
		return true;
	else
		return ThreadManager::getInstance().getCurrentThread()->stopped();
}

Job::Job(Functor* functor, Event& jobFinishedEvent)
:	m_functor(functor)
,	m_jobFinishedEvent(jobFinishedEvent)
,	m_finished(false)
,	m_stopped(false)
{
}

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
	uint32_t cores = OS::getInstance().getCPUCoreCount();
	T_ASSERT (cores > 0);

	m_workerThreads.resize(workerThreads);
	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
	{
		m_workerThreads[i] = ThreadManager::getInstance().create(
			makeFunctor< JobQueue >(
				this,
				&JobQueue::threadWorker,
				int(i)
			).ptr(),
			L"Job queue, worker thread " + toString(i),
			i + 1
		);
		m_workerThreads[i]->start(Thread::Normal);
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
	m_jobQueue.put(job);
	m_jobQueuedEvent.pulse();
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
			m_jobQueue.put(jobs[i]);
		}
		m_jobQueuedEvent.pulse();
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
		if (m_running != 0 && !m_jobFinishedEvent.wait(timeout))
			return false;
	}
	return true;
}

void JobQueue::stop()
{
	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
		m_workerThreads[i]->stop(0);
}

void JobQueue::threadWorker(int id)
{
	Thread* thread = m_workerThreads[id];
	Ref< Job > job;

	for (;;)
	{
		while (m_jobQueue.get(job) && !thread->stopped())
		{
			T_ASSERT (!job->m_finished);
			Atomic::increment(m_running);
			(*job->m_functor)();
			Atomic::decrement(m_running);
			job->m_finished = true;
			job->m_stopped = true;
			m_jobFinishedEvent.broadcast();
		}

		if (thread->stopped())
			break;

		if (!m_jobQueuedEvent.wait(100))
			continue;
	}
}

JobManager& JobManager::getInstance()
{
	static JobManager* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new JobManager();
		SingletonManager::getInstance().addBefore(s_instance, &ThreadManager::getInstance());

		s_instance->m_queue = new JobQueue();
		s_instance->m_queue->create(OS::getInstance().getCPUCoreCount());
	}
	return *s_instance;
}

void JobManager::destroy()
{
	delete this;
}

}
