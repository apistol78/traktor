#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/ThreadManager.h"
#include "Telemetry/AddValueTask.h"
#include "Telemetry/EventTask.h"
#include "Telemetry/SetValueTask.h"
#include "Telemetry/Telemetry.h"

namespace traktor
{
	namespace telemetry
	{
		namespace
		{

const int32_t c_queueErrorLimit = 50;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.telemetry.Telemetry", Telemetry, Object)

Telemetry::Telemetry()
:	m_thread(nullptr)
,	m_queueError(0)
,	m_sequenceNr(0)
{
}

Telemetry& Telemetry::getInstance()
{
	static Telemetry* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new Telemetry();
		s_instance->addRef(nullptr);
		SingletonManager::getInstance().addBefore(s_instance, &ThreadManager::getInstance());
	}
	return *s_instance;
}

bool Telemetry::create(const std::wstring& serverHost, const std::wstring& client, uint32_t sequenceNr)
{
	m_serverHost = serverHost;
	m_client = client;
	m_sequenceNr = sequenceNr;

	// Create queue processing thread.
	m_thread = ThreadManager::getInstance().create(
		[this](){ threadProcessQueue(); },
		L"Telemetry"
	);
	if (!m_thread || !m_thread->start())
	{
		m_thread = nullptr;
		return false;
	}

	return true;
}

void Telemetry::event(const std::wstring& symbol)
{
	if (m_thread)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_queue.push_back(new EventTask(m_serverHost, m_client, m_sequenceNr, symbol));
		m_queueSignal.set();
	}
	Atomic::increment((int32_t&)m_sequenceNr);
}

void Telemetry::set(const std::wstring& symbol, int32_t value)
{
	if (m_thread)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_queue.push_back(new SetValueTask(m_serverHost, m_client, m_sequenceNr, symbol, value));
		m_queueSignal.set();
	}
	Atomic::increment((int32_t&)m_sequenceNr);
}

void Telemetry::add(const std::wstring& symbol, int32_t value)
{
	if (m_thread)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_queue.push_back(new AddValueTask(m_serverHost, m_client, m_sequenceNr, symbol, value));
		m_queueSignal.set();
	}
	Atomic::increment((int32_t&)m_sequenceNr);
}

void Telemetry::flush()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (auto task : m_queue)
		task->execute();
	m_queue.clear();
	m_queueSignal.reset();
}

void Telemetry::destroy()
{
	// Always try to flush all pending tasks.
	flush();

	// Stop processing thread.
	if (m_thread)
	{
		m_thread->stop();
		ThreadManager::getInstance().destroy(m_thread);
		m_thread = nullptr;
	}

	T_SAFE_RELEASE(this);
}

void Telemetry::threadProcessQueue()
{
	Ref< ITask > task;
	while (!m_thread->stopped())
	{
		// Wait for task to be queued.
		if (!m_queueSignal.wait(200))
			continue;

		// Get top task from queue.
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			if (m_queue.empty())
				continue;
			task = m_queue.front();
			T_FATAL_ASSERT (task);
		}

		// Execute task.
		ITask::TaskResult result = task->execute();
		if (result == ITask::TrRetryAgainLater)
		{
			// Failed, keep in queue for later.
			if (++m_queueError >= c_queueErrorLimit)
			{
				T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
				m_queueError = 0;
				m_queue.pop_front();
				if (m_queue.empty())
					m_queueSignal.reset();
			}
			else
				m_thread->sleep(200);
		}
		else
		{
			// Success or catastrophic failure, discard from queue.
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			T_FATAL_ASSERT (!m_queue.empty());
			m_queueError = 0;
			m_queue.pop_front();
			if (m_queue.empty())
				m_queueSignal.reset();
		}
	}
}

	}
}
