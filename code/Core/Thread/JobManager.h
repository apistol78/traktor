#ifndef traktor_JobManager_H
#define traktor_JobManager_H

#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/ThreadsafeFifo.h"
#include "Core/Functor/Functor.h"
#include "Core/Singleton/ISingleton.h"
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

/*! \brief Job handle object.
 * \ingroup Core
 */
class T_DLLCLASS Job
:	public RefCountImpl< IRefCount >
,	public IWaitable
{
public:
	virtual bool wait(int32_t timeout = -1);

private:
	friend class JobManager;

	Ref< Functor > m_functor;
	Event& m_jobFinishedEvent;
	volatile bool m_finished;

	Job(Functor* functor, Event& jobFinishedEvent);
};

/*! \brief Job manager.
 *
 * The job manager is responsible of scheduling
 * enqueued jobs onto any of the available worker threads.
 * As the job manager always has a number of worker threads
 * there is no overhead of having to create threads for each
 * job thus more suitable to smaller tasks.
 */
class T_DLLCLASS JobManager : public ISingleton
{
public:
	static JobManager& getInstance();

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

protected:
	virtual void destroy();

private:
	std::vector< Thread* > m_workerThreads;
	ThreadsafeFifo< Ref< Job > > m_jobQueue;
	Event m_jobQueuedEvent;
	Event m_jobFinishedEvent;

	void threadWorker(int id);

	void create();

	JobManager();

	virtual ~JobManager();
};

}

#endif	// traktor_JobManager_H
