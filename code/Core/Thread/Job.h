#ifndef traktor_Job_H
#define traktor_Job_H

#include "Core/Ref.h"
#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Event;
class Functor;

/*! \brief Job handle object.
 * \ingroup Core
 */
class T_DLLCLASS Job
:	public RefCountImpl< IRefCount >
,	public IWaitable
{
public:
	virtual bool wait(int32_t timeout = -1);

	bool stopped() const;

	void stop() { m_stopped = true; }

	void* operator new (size_t size);

	void operator delete (void* ptr);

	static void join(Ref< Job >& job)
	{
		if (job) { job->wait(); job = 0; }
	}

private:
	friend class JobQueue;

	Ref< Functor > m_functor;
	Event& m_jobFinishedEvent;
	volatile bool m_finished;
	volatile bool m_stopped;

	Job(Functor* functor, Event& jobFinishedEvent);
};

}

#endif	// traktor_Job_H
