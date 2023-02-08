/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/RefArray.h"
#include "Core/Thread/Acquire.h"
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
			[this]() { threadWorker(); },
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

Job* JobQueue::add(const Job::task_t& task)
{
	Ref< Job > job = new Job(m_jobFinishedEvent, task);
	{
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_jobQueueLock);
		m_jobQueue.push_back(job);
		m_pending++;
	}
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
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_jobQueueLock);
		jobs.resize(ntasks);
		for (size_t i = 1; i < ntasks; ++i)
		{
			jobs[i] = new Job(m_jobFinishedEvent, tasks[i]);
			m_jobQueue.push_back(jobs[i]);
			m_pending++;
		}
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
	Ref< Job > job;

	while (!thread->stopped())
	{
		if (m_pending == 0 && !m_jobQueuedEvent.wait(100))
			continue;

		// Pop job from queue.
		{
			T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_jobQueueLock);
			if (!m_jobQueue.empty())
			{
				job = m_jobQueue.front();
				m_jobQueue.pop_front();
			}
			else
				continue;
		}

		// Execute job.
		T_FATAL_ASSERT(job->m_finished == 0);
		job->m_task();
		T_FATAL_ASSERT(job->m_finished == 0);
		job->m_finished = 1;

		// Decrement number of pending jobs and signal anyone waiting for jobs to finish.
		m_pending--;
		m_jobFinishedEvent.broadcast();
	}
}

}
