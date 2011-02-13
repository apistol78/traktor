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
				return false;
		}
		// A job has been finished; check if it this
		// and in such case return true.
	}
	return true;
}

Job::Job(Functor* functor, Event& jobFinishedEvent)
:	m_functor(functor)
,	m_jobFinishedEvent(jobFinishedEvent)
,	m_finished(false)
{
}

JobManager& JobManager::getInstance()
{
	static JobManager* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new JobManager();
		SingletonManager::getInstance().addBefore(s_instance, &ThreadManager::getInstance());
		s_instance->create();
	}
	return *s_instance;
}

Ref< Job > JobManager::add(Functor* functor)
{
	Ref< Job > job = new Job(functor, m_jobFinishedEvent);
	m_jobQueue.put(job);
	m_jobQueuedEvent.pulse();
	return job;
}

void JobManager::fork(const RefArray< Functor >& functors)
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

void JobManager::threadWorker(int id)
{
	Thread* thread = m_workerThreads[id];
	Ref< Job > job;

	for (;;)
	{
		while (m_jobQueue.get(job) && !thread->stopped())
		{
			T_ASSERT (!job->m_finished);
			(*job->m_functor)();
			job->m_finished = true;
			m_jobFinishedEvent.broadcast();
		}

		if (thread->stopped())
			break;

		if (!m_jobQueuedEvent.wait(100))
			continue;
	}
}

void JobManager::create()
{
	uint32_t cores = OS::getInstance().getCPUCoreCount();
	T_ASSERT (cores > 0);

	m_workerThreads.resize(cores);
	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
	{
		m_workerThreads[i] = ThreadManager::getInstance().create(
			makeFunctor< JobManager >(
				this,
				&JobManager::threadWorker,
				int(i)
			).ptr(),
			L"Job worker thread " + toString(i),
			i + 1
		);
		m_workerThreads[i]->start(Thread::Normal);
	}
}

JobManager::JobManager()
{
}

JobManager::~JobManager()
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
}

void JobManager::destroy()
{
	delete this;
}

}
