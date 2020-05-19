#include "Core/Misc/String.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Profiler.h"

namespace traktor
{
	namespace
	{

const size_t c_eventQueueThreshold = 64;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Profiler", Profiler, Object)

Profiler& Profiler::getInstance()
{
	static Profiler* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new Profiler();
		s_instance->addRef(nullptr);
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

void Profiler::setListener(IReportListener* listener)
{
	m_listener = listener;
}

void Profiler::beginEvent(const wchar_t* const name)
{
	if (!m_listener)
		return;

	// Get event queue for calling thread.
	ThreadEvents* te = static_cast< ThreadEvents* >(m_localThreadEvents.get());
	if (!te)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		te = new ThreadEvents();
		te->events.reserve(64);
		m_threadEvents.push_back(te);
		m_localThreadEvents.set(te);
	}

	// Begin event.
	Event& e = te->events.push_back();
	e.name = name;
	e.threadId = ThreadManager::getInstance().getCurrentThread()->id();
	e.depth = uint16_t(te->events.size() - 1);
	e.start = m_timer.getElapsedTime();
	e.end = 0.0;
}

void Profiler::endEvent()
{
	if (!m_listener)
		return;

	// Get event queue for calling thread.
	ThreadEvents* te = static_cast< ThreadEvents* >(m_localThreadEvents.get());
	T_FATAL_ASSERT (te);

	// End event.
	T_FATAL_ASSERT(!te->events.empty());
	Event& e = te->events.back();
	e.end = m_timer.getElapsedTime();

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		// Merge finished event.
		m_events.push_back(e);
		te->events.pop_back();

		// Report events if we've queued enough.
		if (m_events.size() >= c_eventQueueThreshold)
		{
			m_listener->reportProfilerEvents(m_timer.getElapsedTime(), m_events);
			m_events.resize(0);
		}
	}
}

void Profiler::addEvent(const wchar_t* const name, double duration)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	auto& e = m_events.push_back();
	e.name = name;
	e.threadId = -1;
	e.depth = 0;
	e.start = m_timer.getElapsedTime();
	e.end = e.start + duration;

	// Report events if we've queued enough.
	if (m_events.size() >= c_eventQueueThreshold)
	{
		m_listener->reportProfilerEvents(m_timer.getElapsedTime(), m_events);
		m_events.resize(0);
	}
}

double Profiler::getTime() const
{
	return m_timer.getElapsedTime();
}

Profiler::Profiler()
{
	m_timer.start();
}

void Profiler::destroy()
{
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		if (!m_events.empty())
		{
			if (m_listener)
				m_listener->reportProfilerEvents(m_timer.getElapsedTime(), m_events);
			m_events.clear();
		}

		m_listener = nullptr;

		for (auto threadEvent : m_threadEvents)
			delete threadEvent;

		m_threadEvents.clear();
	}
	T_SAFE_RELEASE(this);
}

Profiler::JSONReportListener::JSONReportListener(OutputStream* output)
:	m_output(output)
{
	*m_output << L"[" << Endl;
	*m_output << IncreaseIndent;
}

Profiler::JSONReportListener::~JSONReportListener()
{
	*m_output << DecreaseIndent;
	*m_output << L"]" << Endl;
}

void Profiler::JSONReportListener::reportProfilerEvents(double currentTime, const AlignedVector< Profiler::Event >& events)
{
	for (const auto& event : events)
	{
		*m_output << L"{" << Endl;
		*m_output << IncreaseIndent;
		*m_output << L"\"cat\": \"MAIN\"," << Endl;
		*m_output << L"\"pid\": 0," << Endl;
		*m_output << L"\"tid\": " << event.threadId << L"," << Endl;
		*m_output << L"\"ts\": " << int64_t(event.start * 1000000.0) << L"," << Endl;
		*m_output << L"\"ph\": \"B\"," << Endl;
		*m_output << L"\"name\": \"" << event.name << L"\"," << Endl;
		*m_output << L"\"args\": {}" << Endl;
		*m_output << DecreaseIndent;
		*m_output << L"}," << Endl;
		*m_output << L"{" << Endl;
		*m_output << IncreaseIndent;
		*m_output << L"\"cat\": \"MAIN\"," << Endl;
		*m_output << L"\"pid\": 0," << Endl;
		*m_output << L"\"tid\": " << event.threadId << L"," << Endl;
		*m_output << L"\"ts\": " << int64_t(event.end * 1000000.0) << L"," << Endl;
		*m_output << L"\"ph\": \"E\"," << Endl;
		*m_output << L"\"name\": \"" << event.name << L"\"," << Endl;
		*m_output << L"\"args\": {}" << Endl;
		*m_output << DecreaseIndent;
		*m_output << L"}," << Endl;
	}
}

}
