/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/JobQueue.h"

#include "Core/IRefCount.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Thread/Signal.h"
#include "Core/Thread/ThreadManager.h"

#include <atomic>

namespace traktor
{
namespace
{

class ForkBarrier : public RefCountImpl< IRefCount >
{
public:
	explicit ForkBarrier(int32_t count)
		: m_remaining(count)
	{
	}

	void complete()
	{
		if (--m_remaining == 0)
			m_signal.set();
	}

	void wait()
	{
		Thread* current = ThreadManager::getInstance().getCurrentThread();
		while (m_remaining > 0 && !current->stopped())
			m_signal.wait(100);
	}

private:
	std::atomic< int32_t > m_remaining;
	Signal m_signal;
};

}

struct JobQueue::Worker
{
	ThreadsafeFifo< Job* > queue;
	std::atomic< int32_t > count;
	Event available;

	Worker()
		: count(0)
	{
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.JobQueue", JobQueue, Object)

JobQueue::JobQueue()
	: m_pending(0)
	, m_nextWorker(0)
{
}

JobQueue::~JobQueue()
{
	destroy();
}

bool JobQueue::create(uint32_t workerThreads, Thread::Priority priority)
{
	m_workers.resize(workerThreads);
	for (uint32_t i = 0; i < workerThreads; ++i)
		m_workers[i] = new Worker();

	m_workerThreads.resize(workerThreads);
	for (uint32_t i = 0; i < uint32_t(m_workerThreads.size()); ++i)
	{
		m_workerThreads[i] = ThreadManager::getInstance().create(
			[this, i]() {
			threadWorker(i);
		},
			L"Job queue, worker thread");
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

	for (uint32_t i = 0; i < uint32_t(m_workers.size()); ++i)
		delete m_workers[i];

	m_workers.clear();
}

void JobQueue::enqueue(Job* job)
{
	const uint32_t n = (uint32_t)m_workers.size();
	if (n == 0)
	{
		// No workers to run it; execute inline.
		if (job->m_task)
			job->m_task();
		job->m_finished = true;
		T_SAFE_RELEASE(job);
		m_pending--;
		if (Job::waiters.load() > 0)
			m_jobFinishedEvent.broadcast();
		return;
	}

	Worker* worker = m_workers[m_nextWorker++ % n];
	worker->queue.put(job);
	++worker->count;
	worker->available.pulse();
}

void JobQueue::place(uint32_t slot, Job* job)
{
	Worker* worker = m_workers[slot % (uint32_t)m_workers.size()];
	worker->queue.put(job);
	++worker->count;
}

void JobQueue::wake(uint32_t base, uint32_t count)
{
	const uint32_t n = (uint32_t)m_workers.size();
	const uint32_t woken = (count < n) ? count : n;
	for (uint32_t k = 0; k < woken; ++k)
		m_workers[(base + k) % n]->available.pulse();
}

bool JobQueue::dequeue(uint32_t index, Job*& outJob)
{
	// Our own queue first; uncontended unless somebody is stealing from us.
	if (m_workers[index]->queue.get(outJob))
	{
		--m_workers[index]->count;
		return true;
	}

	// Then steal, starting beside us so idle workers do not all converge on the
	// same victim, and skipping queues which advertise nothing so a failed steal
	// costs one atomic load instead of a lock.
	const uint32_t n = (uint32_t)m_workers.size();
	for (uint32_t i = 1; i < n; ++i)
	{
		Worker* victim = m_workers[(index + i) % n];
		if (victim->count.load() <= 0)
			continue;
		if (victim->queue.get(outJob))
		{
			--victim->count;
			return true;
		}
	}

	return false;
}

Ref< Job > JobQueue::add(const Job::task_t& task)
{
	Ref< Job > job = new Job(m_jobFinishedEvent, task);
	T_SAFE_ADDREF(job);
	m_pending++;
	enqueue(job);
	return job;
}

void JobQueue::add(const Job::task_t* tasks, size_t ntasks)
{
	if (ntasks == 0)
		return;

	const uint32_t count = (uint32_t)ntasks;
	const uint32_t n = (uint32_t)m_workers.size();

	// Counted before handing anything over, since enqueue decrements again on
	// the no-worker path.
	m_pending += (int32_t)count;

	if (n == 0)
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			Job* job = new Job(m_jobFinishedEvent, tasks[i]);
			T_SAFE_ADDREF(job);
			enqueue(job);
		}
		return;
	}

	// Spread the batch out first, then wake; see wake().
	const uint32_t base = m_nextWorker.fetch_add(count);

	for (uint32_t i = 0; i < count; ++i)
	{
		Job* job = new Job(m_jobFinishedEvent, tasks[i]);
		T_SAFE_ADDREF(job);
		place(base + i, job);
	}

	wake(base, count);
}

void JobQueue::fork(const Job::task_t* tasks, size_t ntasks)
{
	if (ntasks == 0)
		return;

	Ref< ForkBarrier > latch;

	// Create jobs for given functors.
	if (ntasks > 1)
	{
		const uint32_t njobs = (uint32_t)(ntasks - 1);
		const uint32_t n = (uint32_t)m_workers.size();

		latch = new ForkBarrier((int32_t)njobs);

		// Counted before handing anything over, since enqueue decrements again on
		// the no-worker path.
		m_pending += (int32_t)njobs;

		const auto makeJob = [&](size_t index) -> Job* {
			const Job::task_t task = tasks[index];
			Job* job = new Job(m_jobFinishedEvent, [task, latch]() {
				task();
				latch->complete();
			});
			T_SAFE_ADDREF(job);
			return job;
		};

		if (n == 0)
		{
			for (uint32_t j = 0; j < njobs; ++j)
				enqueue(makeJob(j + 1));
		}
		else
		{
			const uint32_t base = m_nextWorker.fetch_add(njobs);

			for (uint32_t j = 0; j < njobs; ++j)
				place(base + j, makeJob(j + 1));

			wake(base, njobs);
		}
	}

	// Execute first functor on caller thread.
	tasks[0]();

	if (latch)
		latch->wait();
}

bool JobQueue::wait(int32_t timeout)
{
	++Job::waiters;

	bool result = true;
	while (m_pending > 0)
	{
		if (!m_jobFinishedEvent.wait(timeout))
		{
			result = false;
			break;
		}
	}

	--Job::waiters;
	return result;
}

bool JobQueue::waitCurrent(int32_t timeout)
{
	++Job::waiters;
	const bool result = (m_pending > 0) ? m_jobFinishedEvent.wait(timeout) : true;
	--Job::waiters;
	return result;
}

void JobQueue::stop()
{
	for (uint32_t i = 0; i < (uint32_t)m_workerThreads.size(); ++i)
		m_workerThreads[i]->stop(0);

	for (uint32_t i = 0; i < (uint32_t)m_workers.size(); ++i)
		m_workers[i]->available.broadcast();

	for (uint32_t i = 0; i < (uint32_t)m_workerThreads.size(); ++i)
		m_workerThreads[i]->stop();
}

void JobQueue::threadWorker(uint32_t index)
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();
	Worker* self = m_workers[index];
	Job* job;

	while (!thread->stopped())
	{
		// Try our own queue, then steal.
		if (!dequeue(index, job))
		{
			self->available.wait(100);
			continue;
		}

		// Execute job.
		auto task = job->m_task;
		if (task)
			task();
		job->m_finished = true;
		T_SAFE_RELEASE(job);

		// Decrement number of pending jobs and signal anyone waiting for jobs.
		m_pending--;
		if (Job::waiters.load() > 0)
			m_jobFinishedEvent.broadcast();
	}
}

}
