#ifndef traktor_Profiler_H
#define traktor_Profiler_H

#include <map>
#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \ingroup Core */
//@{

class Thread;
class OutputStream;

/*! \brief Runtime profiler.
 *
 * The runtime profiler measures time spent in
 * scopes.
 */
class T_DLLCLASS Profiler : public Object
{
	T_RTTI_CLASS(Profiler)

public:
	typedef void* Handle;

	static Profiler& getInstance();

	Handle enterScope(const wchar_t* const name);

	void leaveScope(Handle handle);

	void report(OutputStream& os);

private:
	struct Report
	{
		typedef std::map< const wchar_t*, Report > Map;

		Report* parent;
		uint32_t count;
		double time;
		double peek;
		double last;
		Map reports;
	};

	Timer m_timer;
	std::map< Thread*, Report > m_globalReports;
	std::map< Thread*, Report* > m_currentReports;

	void report(OutputStream& os, const std::wstring& name, Report& r);

	void pushReport(Report& report);

	void popReport();

	Report& getReport();
};

class ProfilerScoped
{
public:
	ProfilerScoped(const wchar_t* const name)
	: m_handle(Profiler::getInstance().enterScope(name))
	{
	}

	~ProfilerScoped()
	{
		Profiler::getInstance().leaveScope(m_handle);
	}

private:
	Profiler::Handle m_handle;
};

// Uncomment this line if you want scope profiling enabled.
//#define T_PROFILER_ENABLE

#if defined(T_PROFILER_ENABLE)

#define WIDEN2__(x) L ## #x
#define WIDEN__(x) WIDEN2__(x)

#define T_PROFILER_SCOPE(name) \
	ProfilerScoped __profiler_ ## name ## _ ( WIDEN__( name ) );
#endif

#if !defined(T_PROFILER_SCOPE)
#define T_PROFILER_SCOPE(name)
#endif

//@}

}

#endif	// traktor_Profiler_H
