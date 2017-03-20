#ifndef traktor_Profiler_H
#define traktor_Profiler_H

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

// Uncomment this line if you want scope profiling enabled.
#define T_PROFILER_ENABLE

/*! \ingroup Core */
//@{

class Thread;
class OutputStream;

/*! \brief Runtime profiler.
 * \ingroup Core
 *
 * The runtime profiler measures time spent in
 * scopes.
 */
class T_DLLCLASS Profiler : public Object
{
	T_RTTI_CLASS;

public:
	struct Event
	{
		const wchar_t* name;
		uint32_t threadId;
		double start;
		double end;
	};

	/*! \brief Profiler report listener.
	 */
	class IReportListener : public IRefCount
	{
	public:
		virtual void reportProfilerEvents(const AlignedVector< Event >& events) = 0;
	};

	/*! \brief JSON report listener. 
	 *
	 * Format of JSON is Chromium tracer compatible.
	 */
	class JSONReportListener : public RefCountImpl< IReportListener >
	{
	public:
		JSONReportListener(OutputStream* output);

		void flush();

		virtual void reportProfilerEvents(const AlignedVector< Event >& events) T_OVERRIDE T_FINAL;

	private:
		Ref< OutputStream > m_output;
	};

	typedef void* handle_t;

	static Profiler& getInstance();

	/*! \brief Set report listener.
	 */
	void setListener(IReportListener* listener);

	/*! \brief Begin recording event.
	 */
	handle_t beginEvent(const wchar_t* const name);

	/*! \brief End recording event.
	 */
	void endEvent(handle_t handle);

private:
	Ref< IReportListener > m_listener;
	Semaphore m_lock;
	AlignedVector< Event > m_events;
	Timer m_timer;
};

/*! \brief Scoped profiling event.
 * \ingroup Core
 */
class ProfilerScoped
{
public:
	ProfilerScoped(const wchar_t* const name)
	:	m_handle(Profiler::getInstance().beginEvent(name))
	{
	}

	~ProfilerScoped()
	{
		Profiler::getInstance().endEvent(m_handle);
	}

private:
	Profiler::handle_t m_handle;
};

#if defined(T_PROFILER_ENABLE)
#	define T_PROFILER_SCOPE(name) \
		T_ANONYMOUS_VAR(ProfilerScoped)(name);
#endif
#if !defined(T_PROFILER_SCOPE)
#	define T_PROFILER_SCOPE(name)
#endif

//@}

}

#endif	// traktor_Profiler_H
