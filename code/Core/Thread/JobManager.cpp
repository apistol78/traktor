#include "Core/Platform.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Atomic.h"
#include "Core/System/OS.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Misc/String.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Job", Job, Singleton)

Job::Job(Functor* functor)
:	m_functor(functor)
,	m_signalFinished(0)
{
}

Job::~Job()
{
	delete m_functor;
}

void Job::wait()
{
	Signal* signalFinished = m_signalFinished;
	if (signalFinished)
		signalFinished->wait();
}

void Job::execute()
{
	T_ASSERT (m_functor);
	(*m_functor)();

	Signal* signalFinished = Atomic::exchange< Signal* >(m_signalFinished, 0);
	if (signalFinished)
		signalFinished->set();
}

Job& Job::operator = (Functor* functor)
{
	if (m_functor)
	{
		T_ASSERT (m_functor != functor);
		delete m_functor;
	}

	m_functor = functor;
	m_signalFinished = 0;

	return *this;
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.JobManager", JobManager, Singleton)

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
	Acquire< Semaphore > __lock__(m_queueLock);

	uint32_t index = m_write++ & (MaxQueuedJobs - 1);
	
	// Spin lock until slot is empty.
	if (m_queue[index])
	{
		Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
		do
		{
			Release< Semaphore > __unlock__(m_queueLock);
			currentThread->sleep(0);
		}
		while (m_queue[index]);
	}

	// Grab signal from buffer.
	job.m_signalFinished = &m_signalBuffer[index];
	job.m_signalFinished->reset();

	m_queue[index] = &job;

	m_eventJobQueued.pulse();
}

void JobManager::fork(Job* jobs, int count)
{
	if (count > 1)
	{
		Acquire< Semaphore > __lock__(m_queueLock);

		for (int i = 1; i < count; ++i)
		{
			uint32_t index = m_write++ & (MaxQueuedJobs - 1);
			T_ASSERT (!m_queue[index]);

			// Grab signal from buffer.
			jobs[i].m_signalFinished = &m_signalBuffer[index];
			jobs[i].m_signalFinished->reset();

			m_queue[index] = &jobs[i];
		}

		// Launch jobs.
		m_eventJobQueued.pulse(count - 1);
	}

	// Execute first job in caller thread.
	jobs[0].execute();

	for (int i = 1; i < count; ++i)
		jobs[i].wait();
}

void JobManager::threadWorker(int id)
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();
	uint32_t index;
	Job* job;

	while (!thread->stopped())
	{
		if (!m_eventJobQueued.wait(300))
			continue;
		
		{
			Acquire< Semaphore > __lock__(m_queueLock);
			index = m_read++ & (MaxQueuedJobs - 1);
			job = m_queue[index];
		}

		if (job)
			job->execute();

		{
			Acquire< Semaphore > __lock__(m_queueLock);
			m_queue[index] = 0;
		}
	}
}

JobManager::JobManager()
:	m_write(0)
,	m_read(0)
{
	for (uint32_t i = 0; i < MaxQueuedJobs; ++i)
		m_queue[i] = 0;

	uint32_t cores = OS::getInstance().getCPUCoreCount();
	T_ASSERT (cores > 0);

	m_threadWorkers.resize(cores);
	for (uint32_t i = 0; i < uint32_t(m_threadWorkers.size()); ++i)
	{
		m_threadWorkers[i] = ThreadManager::getInstance().create(makeFunctor< JobManager >(this, &JobManager::threadWorker, int(i)), L"Job worker thread " + toString(i), i + 1);
		m_threadWorkers[i]->start(Thread::Above);
	}
}

JobManager::~JobManager()
{
	for (uint32_t i = 0; i < uint32_t(m_threadWorkers.size()); ++i)
	{
		m_threadWorkers[i]->stop();
		ThreadManager::getInstance().destroy(m_threadWorkers[i]);
	}
}

void JobManager::destroy()
{
	delete this;
}

}
