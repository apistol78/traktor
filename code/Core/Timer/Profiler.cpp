#include "Core/Timer/Profiler.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Profiler", Profiler, Object)

Profiler& Profiler::getInstance()
{
	static Profiler instance;
	return instance;
}

Profiler::Handle Profiler::enterScope(const wchar_t* const name)
{
	Report& currentReport = getReport();
	Report::Map::iterator i = currentReport.reports.find(name);

	if (i != currentReport.reports.end())
	{
		i->second.count++;
		i->second.last = m_timer.getElapsedTime();
		pushReport(i->second);
		return (Profiler::Handle)&i->second;
	}

	Report& r = currentReport.reports[name];

	r.parent = &currentReport;
	r.count = 1;
	r.time = 0.0;
	r.peek = 0.0;
	r.last = m_timer.getElapsedTime();

	pushReport(r);

	return (Profiler::Handle)&r;
}

void Profiler::leaveScope(Handle handle)
{
	Report* r = reinterpret_cast< Report* >(handle);
	double deltaTime = m_timer.getElapsedTime() - r->last;
	r->time += deltaTime;
	r->peek = std::max(r->peek, deltaTime);
	popReport();
}

void Profiler::report(OutputStream& os)
{
	os << L"<?xml version=\"1.0\" encoding=\"utf-8\"?>" << Endl;
	os << L"<profile>" << Endl;
	os << IncreaseIndent;

	for (std::map< Thread*, Report >::iterator i = m_globalReports.begin(); i != m_globalReports.end(); ++i)
	{
		std::wstring name = L"Thread " + toString(i->first);
		report(os, name, i->second);
	}

	os << DecreaseIndent;
	os << L"</profile>" << Endl;
}

void Profiler::report(OutputStream& os, const std::wstring& name, Report& r)
{
	double ms = r.time * 1000.0;
	double mspc = ms / r.count;
	double peek = r.peek * 1000.0;
	double mspc2 = r.count > 1 ? (ms - peek) / (r.count - 1) : mspc;

	os << L"<report" << Endl;
	os << L"\tid=\"" << name << L"\"" << Endl;
	os << L"\ttime=\"" << ms << L"\"" << Endl;
	os << L"\tcount=\"" << r.count << L"\"" << Endl;
	os << L"\tmspc=\"" << mspc << L"\"" << Endl;
	os << L"\tmspc-peek=\"" << peek << L"\"" << Endl;
	os << L"\tmspc-without-peek=\"" << mspc2 << L"\"" << Endl;
	os << L">" << Endl;

	os << IncreaseIndent;
	for (Report::Map::iterator i = r.reports.begin(); i != r.reports.end(); ++i)
		report(os, i->first, i->second);
	os << DecreaseIndent;

	os << L"</report>" << Endl;
}

void Profiler::pushReport(Report& report_)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	report_.parent = m_currentReports[currentThread];
	m_currentReports[currentThread] = &report_;
}

void Profiler::popReport()
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	m_currentReports[currentThread] = m_currentReports[currentThread]->parent;
}

Profiler::Report& Profiler::getReport()
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();

	std::map< Thread*, Report* >::iterator i = m_currentReports.find(currentThread);
	if (i != m_currentReports.end())
		return *i->second;

	Report& r = m_globalReports[currentThread];

	r.parent = 0;
	r.count = 0;
	r.time = 0.0;
	r.last = 0.0;

	m_currentReports[currentThread] = &r;

	return r;
}

}
