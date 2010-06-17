#include "Core/Platform.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Atomic.h"
#include "Core/System/OS.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Misc/String.h"

namespace traktor
{

Job::Job(Functor* functor)
:	m_functor(functor)
,	m_finished(1)
{
}

Job::~Job()
{
	delete m_functor;
}

void Job::begin()
{
	m_finished = 0;
}

void Job::execute()
{
	T_ASSERT (!m_finished);
	if (m_functor)
	{
		(*m_functor)();
		m_finished = 1;
	}
}

bool Job::wait(int32_t timeout)
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();
	while (m_finished == 0)
		thread->yield();
	return true;
}

Job& Job::operator = (Functor* functor)
{
	if (m_functor)
	{
		T_ASSERT (m_functor != functor);
		delete m_functor;
	}

	m_functor = functor;
	m_finished = 1;

	return *this;
}

JobManager& JobManager::getInstance()
{
	static JobManager* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new JobManager();
		SingletonManager::getInstance().addBefore(s_instance, &ThreadManager::getInstance());
	}
	return *s_instance;
}

void JobManager::add(Job& job)
{
	job.begin();
	m_jobQueue.put(&job);
	m_jobQueueEvent.pulse();
}

void JobManager::fork(Job* jobs, int count)
{
	if (count > 1)
	{
		for (int i = 1; i < count; ++i)
		{
			jobs[i].begin();
			m_jobQueue.put(&jobs[i]);
		}

		m_jobQueueEvent.pulse(count - 1);
	}

	jobs[0].begin();
	jobs[0].execute();

	for (int i = 1; i < count; ++i)
		jobs[i].wait();
}

void JobManager::threadWorker(int id)
{
	Thread* thread = m_workerThreads[id];
	Job* job;

	while (!thread->stopped())
	{
		while (m_jobQueue.get(job))
			job->execute();

		if (!m_jobQueueEvent.wait(100))
			continue;
	}
}

JobManager::JobManager()
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
			),
			L"Job worker thread " + toString(i),
			i + 1
		);
		m_workerThreads[i]->start();
	}
}

JobManager::~JobManager()
{
	// Destroy worker threads.
	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
	{
		m_workerThreads[i]->stop();
		ThreadManager::getInstance().destroy(m_workerThreads[i]);
	}

	// Ensure all pending jobs are dropped.
	Job* job;
	while (m_jobQueue.get(job))
	{
		T_ASSERT (job);
		delete job->m_functor;
		job->m_finished = true;
	}
}

void JobManager::destroy()
{
	delete this;
}

}
