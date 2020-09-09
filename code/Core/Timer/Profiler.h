#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/SpinLock.h"
#include "Core/Thread/ThreadLocal.h"
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
#if !defined(__ANDROID__)
#	define T_PROFILER_ENABLE
#endif

/*! \ingroup Core */
//@{

class Thread;
class OutputStream;

/*! Runtime profiler.
 * \ingroup Core
 *
 * The runtime profiler measures time spent in
 * scopes.
 */
class T_DLLCLASS Profiler
:	public Object
,	public ISingleton
{
	T_RTTI_CLASS;

public:
	struct Event
	{
		uint16_t name;
		uint32_t threadId;
		uint16_t depth;
		double start;
		double end;
	};

	/*! Profiler report listener.
	 */
	class IReportListener : public IRefCount
	{
	public:
		virtual void reportProfilerDictionary(const SmallMap< uint16_t, std::wstring >& dictionary) = 0;

		virtual void reportProfilerEvents(double currentTime, const AlignedVector< Event >& events) = 0;
	};

	typedef void* handle_t;

	static Profiler& getInstance();

	/*! Set report listener.
	 */
	void setListener(IReportListener* listener);

	/*! Begin recording event.
	 */
	void beginEvent(const std::wstring& name);

	/*! End recording event.
	 */
	void endEvent();

	/*! Add manual event. */
	void addEvent(const std::wstring& name, double start, double duration);

	/*! Get current time.
	 */
	double getTime() const;

protected:
	Profiler();

	virtual void destroy() override final;

private:
	struct ThreadEvents
	{
		AlignedVector< Event > events;
	};

	Ref< IReportListener > m_listener;
	Semaphore m_lock;
	SpinLock m_nameIdsLock;
	SmallMap< std::wstring, uint16_t > m_nameIds;
	SmallMap< uint16_t, std::wstring > m_dictionary;
	bool m_dictionaryDirty;
	AlignedVector< Event > m_events;
	AlignedVector< ThreadEvents* > m_threadEvents;
	ThreadLocal m_localThreadEvents;
	Timer m_timer;
};

/*! Scoped profiling event.
 * \ingroup Core
 */
class ProfilerScoped
{
public:
	ProfilerScoped(const std::wstring& name)
	{
		Profiler::getInstance().beginEvent(name);
	}

	~ProfilerScoped()
	{
		Profiler::getInstance().endEvent();
	}
};

#if defined(T_PROFILER_ENABLE)
#	define T_PROFILER_BEGIN(name)	{ Profiler::getInstance().beginEvent(name); }
#	define T_PROFILER_END()			{ Profiler::getInstance().endEvent(); }
#	define T_PROFILER_SCOPE(name)	T_ANONYMOUS_VAR(ProfilerScoped)(name);
#else
#	define T_PROFILER_BEGIN(name)	{}
#	define T_PROFILER_END()			{}
#	define T_PROFILER_SCOPE(name)
#endif

//@}

}

