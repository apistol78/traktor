#ifndef traktor_JobManager_H
#define traktor_JobManager_H

#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/JobQueue.h"

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

/*! \brief Job manager.
 * \ingroup Core
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
	Ref< Job > add(Functor* functor) { return m_queue.add(functor); }

	/*! \brief Enqueue jobs and wait for all to finish.
	 *
	 * Add jobs to internal worker queue, one job
	 * is always run on the caller thread to reduce
	 * work for kernel scheduler.
	 */
	void fork(const RefArray< Functor >& functors) { return m_queue.fork(functors); }

	/*! \brief Wait until all jobs are finished.
	 *
	 * \param timeout Timeout in milliseconds; -1 if infinite timeout.
	 * \return True if jobs have finished, false if timeout.
	 */
	bool wait(int32_t timeout = -1) { return m_queue.wait(timeout); }

	/*! \brief Stop all worker threads. */
	void stop() { m_queue.stop(); }

protected:
	virtual void destroy();

private:
	JobQueue m_queue;
};

}

#endif	// traktor_JobManager_H
