#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Profiler.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Profiler", Profiler, Object)

Profiler& Profiler::getInstance()
{
	static Profiler instance;
	return instance;
}

void Profiler::setListener(IReportListener* listener)
{
	m_listener = listener;
}

Profiler::handle_t Profiler::beginEvent(const wchar_t* const name)
{
	//if (m_listener)
	//{
	//	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	//	size_t index = m_events.size();
	//	Event& e = m_events.push_back();
	//	e.name = name;
	//	e.threadId = ThreadManager::getInstance().getCurrentThread()->id();
	//	e.start = m_timer.getElapsedTime();
	//	e.end = 0.0;
	//	return (Profiler::handle_t)index;
	//}
	//else
		return 0;
}

void Profiler::endEvent(handle_t handle)
{
	//if (m_listener)
	//{
	//	Event& e = m_events[(size_t)handle];
	//	e.end = m_timer.getElapsedTime();
	//}
}

Profiler::JSONReportListener::JSONReportListener(OutputStream* output)
:	m_output(output)
{
	*m_output << L"[" << Endl;
	*m_output << IncreaseIndent;
}

void Profiler::JSONReportListener::flush()
{
	*m_output << DecreaseIndent;
	*m_output << L"]" << Endl;
}

void Profiler::JSONReportListener::reportProfilerEvents(const AlignedVector< Profiler::Event >& events)
{
	for (AlignedVector< Profiler::Event >::const_iterator i = events.begin(); i != events.end(); )
	{
		*m_output << L"{" << Endl;
		*m_output << IncreaseIndent;
		*m_output << L"\"cat\": \"MAIN\"," << Endl;
		*m_output << L"\"pid\": 0," << Endl;
		*m_output << L"\"tid\": " << i->threadId << L"," << Endl;
		*m_output << L"\"ts\": " << int64_t(i->start * 1000.0) << L"," << Endl;
		*m_output << L"\"ph\": \"B\"," << Endl;
		*m_output << L"\"name\": \"" << i->name << L"\"," << Endl;
		*m_output << L"\"args\": {}" << Endl;
		*m_output << DecreaseIndent;
		*m_output << L"}," << Endl;
		*m_output << L"{" << Endl;
		*m_output << IncreaseIndent;
		*m_output << L"\"cat\": \"MAIN\"," << Endl;
		*m_output << L"\"pid\": 0," << Endl;
		*m_output << L"\"tid\": " << i->threadId << L"," << Endl;
		*m_output << L"\"ts\": " << int64_t(i->end * 1000.0) << L"," << Endl;
		*m_output << L"\"ph\": \"E\"," << Endl;
		*m_output << L"\"name\": \"" << i->name << L"\"," << Endl;
		*m_output << L"\"args\": {}" << Endl;
		*m_output << DecreaseIndent;
		*m_output << L"}," << Endl;
	}
}

}
