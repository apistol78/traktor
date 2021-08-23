#pragma once

#include <functional>
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

/*! Job handle object.
 * \ingroup Core
 */
class T_DLLCLASS Job
:	public RefCountImpl< IRefCount >
,	public IWaitable
{
public:
	typedef std::function< void() > task_t;

	virtual bool wait(int32_t timeout = -1) override final;

	void stop();

	bool stopped() const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	friend class JobQueue;

	Event& m_jobFinishedEvent;
	task_t m_task;
	uint32_t m_finished;

	explicit Job(Event& jobFinishedEvent, const task_t& task);

	Job() = delete;

	Job(const Job&) = delete;

	Job(Job&&) = delete;
};

}
