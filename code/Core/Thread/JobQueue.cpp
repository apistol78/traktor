/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/RefArray.h"
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
			[=, this]() { threadWorker(); },
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

Ref< Job > JobQueue::add(const Job::task_t& task)
{
	Ref< Job > job = new Job(m_jobFinishedEvent, task);
	T_SAFE_ADDREF(job);
	m_jobQueue.put(job);
	m_pending++;
	m_jobQueuedEvent.pulse();
	return job;
}

void JobQueue::fork(const Job::task_t* tasks, size_t ntasks)
{
	RefArray< Job > jobs;

	if (ntasks == 0)
		return;

	// Create jobs for given functors.
	if (ntasks > 1)
	{
		jobs.resize(ntasks);
		{
			for (size_t i = 1; i < ntasks; ++i)
			{
				jobs[i] = new Job(m_jobFinishedEvent, tasks[i]);
				T_SAFE_ADDREF(jobs[i]);
				m_jobQueue.put(jobs[i]);
			}
		}
		m_pending += ntasks - 1;
		m_jobQueuedEvent.pulse((int32_t)(ntasks - 1));
	}

	// Execute first functor on caller thread.
	tasks[0]();

	// Wait until all jobs has finished.
	for (uint32_t i = 1; i < jobs.size(); )
	{
		if (jobs[i]->wait())
			++i;
	}
}

bool JobQueue::wait(int32_t timeout)
{
	while (m_pending > 0)
	{
		if (!m_jobFinishedEvent.wait(timeout))
			return false;
	}
	return true;
}

bool JobQueue::waitCurrent(int32_t timeout)
{
	return m_pending > 0 ? m_jobFinishedEvent.wait(timeout) : true;
}

void JobQueue::stop()
{
	for (uint32_t i = 0; i < (uint32_t)m_workerThreads.size(); ++i)
		m_workerThreads[i]->stop(0);
	for (uint32_t i = 0; i < (uint32_t)m_workerThreads.size(); ++i)
		m_workerThreads[i]->stop();
}

void JobQueue::threadWorker()
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();
	Job* job;

	while (!thread->stopped())
	{
		// Try to get a job from the queue.
		if (!m_jobQueue.get(job))
		{
			m_jobQueuedEvent.wait(100);
			continue;			
		}

		// Execute job.
		if (job->m_task)
			job->m_task();
		job->m_finished = true;
		T_SAFE_RELEASE(job);

		// Decrement number of pending jobs and signal anyone waiting for jobs to finish.
		m_pending--;
		m_jobFinishedEvent.broadcast();
	}
}

}
