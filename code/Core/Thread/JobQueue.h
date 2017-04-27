/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_JobQueue_H
#define traktor_JobQueue_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/ThreadsafeFifo.h"
#include "Core/Thread/Event.h"
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

class Job;
class Functor;

/*! \brief Job queue.
 * \ingroup Core
 */
class T_DLLCLASS JobQueue : public Object
{
	T_RTTI_CLASS;

public:
	JobQueue();

	virtual ~JobQueue();

	/*! \brief Create queue.
	 *
	 * \param workerThreads Number of worker threads.
	 * \return True if successfully created.
	 */
	bool create(uint32_t workerThreads, Thread::Priority priority);

	/*! \brief Destroy queue. */
	void destroy();

	/*! \brief Enqueue job.
	 *
	 * Add job to internal worker queue, as soon as
	 * a worker thread is idle the scheduler assigns
	 * a new job to that thread from this queue.
	 */
	Ref< Job > add(Functor* functor);

	/*! \brief Enqueue jobs and wait for all to finish.
	 *
	 * Add jobs to internal worker queue, one job
	 * is always run on the caller thread to reduce
	 * work for kernel scheduler.
	 */
	void fork(const RefArray< Functor >& functors);

	/*! \brief Wait until all jobs are finished.
	 *
	 * \param timeout Timeout in milliseconds; -1 if infinite timeout.
	 * \return True if jobs have finished, false if timeout.
	 */
	bool wait(int32_t timeout = -1);

	/*! \brief Stop all worker threads. */
	void stop();

	/*! \brief Number of pending jobs. */
	int32_t getPendingCount() const;

	/*! \brief Number of running jobs. */
	int32_t getRunningCount() const;

private:
	std::vector< Thread* > m_workerThreads;
	ThreadsafeFifo< Ref< Job > > m_jobQueue;
	Event m_jobQueuedEvent;
	Event m_jobFinishedEvent;
	int32_t m_running;

	void threadWorker();
};

}

#endif	// traktor_JobQueue_H
