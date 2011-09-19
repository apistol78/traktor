#include "Core/Thread/Event.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{

bool Job::wait(int32_t timeout)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!m_finished && !currentThread->stopped())
	{
		if (!m_jobFinishedEvent.wait(timeout >= 0 ? timeout : 100))
		{
			if (timeout >= 0)
				return m_finished;
		}
		// A job has been finished; check if it this
		// and in such case return true.
	}
	return true;
}

bool Job::stopped() const
{
	if (m_stopped || m_finished)
		return true;
	else
		return ThreadManager::getInstance().getCurrentThread()->stopped();
}

Job::Job(Functor* functor, Event& jobFinishedEvent)
:	m_functor(functor)
,	m_jobFinishedEvent(jobFinishedEvent)
,	m_finished(false)
,	m_stopped(false)
{
}

}
