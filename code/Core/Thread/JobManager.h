#ifndef traktor_JobManager_H
#define traktor_JobManager_H

#include "Core/Singleton/Singleton.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Signal.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Functor/Functor.h"

namespace traktor
{

/*! \brief Job container.
 * \ingroup Core
 *
 * Each job is maintained by this class.
 */
class T_DLLCLASS Job : public Object
{
	T_RTTI_CLASS(Job)

public:
	Job(Functor* functor = 0);

	virtual ~Job();

	/*! \brief Wait until finished.
	 *
	 * Block caller thread until job is finished.
	 */
	void wait();

	/*! \brief Execute job in caller thread. */
	void execute();

	Job& operator = (Functor* functor);

	/*! \brief Return true if job has been enqueued in the job manager. */
	inline bool enqueued() const
	{
		return bool(m_signalFinished != 0);
	}

private:
	friend class JobManager;

	Functor* m_functor;
	Signal* m_signalFinished;
};

/*! \brief Job manager.
 *
 * The job manager is responsible of scheduling
 * enqueued jobs onto any of the available worker threads.
 * As the job manager always has a number of worker threads
 * there is no overhead of having to create threads for each
 * job thus more suitable to smaller tasks.
 */
class T_DLLCLASS JobManager : public Singleton
{
	T_RTTI_CLASS(JobManager)

public:
	static JobManager& getInstance();

	/*! \brief Enqueue job.
	 *
	 * Add job to internal worker queue, as soon as
	 * a worker thread is idle the scheduler assigns
	 * a new job to that thread from this queue.
	 *
	 * \note
	 * As the queue only stores a pointer to the job
	 * it's important to remember to keep the job object
	 * alive until it's finished.
	 */
	void add(Job& job);

	/*! \brief Enqueue jobs and wait for all to finish.
	 *
	 * Add jobs to internal worker queue, one job
	 * is always run on the caller thread to reduce
	 * work for kernel scheduler.
	 */
	void fork(Job* jobs, int count);

protected:
	virtual void destroy();

private:
	enum { MaxQueuedJobs = 256 };

	struct Slot
	{
		Job* job;
		Signal* signal;
	};

	std::vector< Thread* > m_threadWorkers;
	Job* m_queue[MaxQueuedJobs];
	uint32_t m_write;
	uint32_t m_read;
	Event m_eventJobQueued;
	Signal m_signalBuffer[MaxQueuedJobs];
	Semaphore m_queueLock;

	void threadWorker(int id);

	JobManager();

	~JobManager();
};

}

#endif	// traktor_JobManager_H
