#include "Core/Functor/Functor.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadManager.h"
#include "Telemetry/AddValueTask.h"
#include "Telemetry/SetValueTask.h"
#include "Telemetry/Telemetry.h"

namespace traktor
{
	namespace telemetry
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.telemetry.Telemetry", Telemetry, Object)

Telemetry::Telemetry()
:	m_thread(0)
{
}

Telemetry& Telemetry::getInstance()
{
	static Telemetry* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new Telemetry();
		s_instance->addRef(0);
		SingletonManager::getInstance().addBefore(s_instance, &ThreadManager::getInstance());
	}
	return *s_instance;
}

bool Telemetry::create(const std::wstring& serverHost, const std::wstring& client)
{
	m_serverHost = serverHost;
	m_client = client;

	m_thread = ThreadManager::getInstance().create(
		makeFunctor(this, &Telemetry::threadProcessQueue),
		L"Telemetry"
	);
	if (!m_thread || !m_thread->start())
	{
		m_thread = 0;
		return false;
	}

	return true;
}

void Telemetry::set(const std::wstring& symbol, int32_t value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_queue.push_back(new SetValueTask(m_serverHost, m_client, symbol, value));
	m_queueSignal.set();
}

void Telemetry::add(const std::wstring& symbol, int32_t value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_queue.push_back(new AddValueTask(m_serverHost, m_client, symbol, value));
	m_queueSignal.set();
}

void Telemetry::flush()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (RefArray< ITask >::iterator i = m_queue.begin(); i != m_queue.end(); ++i)
		(*i)->execute();
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
		m_thread = 0;
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

		// Get task from queue.
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			if (m_queue.empty())
				continue;
			task = m_queue.front();
			T_FATAL_ASSERT (task);
		}

		// Execute task.
		if (task->execute())
		{
			// Successful, discard from queue.
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			T_FATAL_ASSERT (!m_queue.empty());
			m_queue.pop_front();
			if (m_queue.empty())
				m_queueSignal.reset();
		}
		else
		{
			// Task failed, keep in queue and retry a bit later.
			m_thread->sleep(200);
		}
	}
}

	}
}
