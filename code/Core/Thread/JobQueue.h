/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/ThreadsafeFifo.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
#include "Core/Thread/Thread.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Job queue.
 * \ingroup Core
 */
class T_DLLCLASS JobQueue : public Object
{
	T_RTTI_CLASS;

public:
	JobQueue();

	virtual ~JobQueue();

	/*! Create queue.
	 *
	 * \param workerThreads Number of worker threads.
	 * \return True if successfully created.
	 */
	bool create(uint32_t workerThreads, Thread::Priority priority);

	/*! Destroy queue. */
	void destroy();

	/*! Enqueue job.
	 *
	 * Add job to internal worker queue, as soon as
	 * a worker thread is idle the scheduler assigns
	 * a new job to that thread from this queue.
	 */
	Ref< Job > add(const Job::task_t& task);

	/*! Enqueue jobs and wait for all to finish.
	 *
	 * Add jobs to internal worker queue, one job
	 * is always run on the caller thread to reduce
	 * work for kernel scheduler.
	 */
	void fork(const Job::task_t* tasks, size_t ntasks);

	/*! Wait until all jobs are finished.
	 *
	 * \param timeout Timeout in milliseconds; -1 if infinite timeout.
	 * \return True if jobs have finished, false if timeout.
	 */
	bool wait(int32_t timeout = -1);

	/*! Wait until current job is finished.
	 *
	 * \param timeout Timeout in milliseconds; -1 if infinite timeout.
	 * \return True if jobs have finished, false if timeout.
	 */
	bool waitCurrent(int32_t timeout = -1);

	/*! Stop all worker threads. */
	void stop();

private:
	AlignedVector< Thread* > m_workerThreads;
	ThreadsafeFifo< Job* > m_jobQueue;
	Event m_jobQueuedEvent;
	Event m_jobFinishedEvent;
	std::atomic< int32_t > m_pending;

	void threadWorker();
};

}

