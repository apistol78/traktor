#include "Core/Functor/Functor.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadManager.h"
#include "Telemetry/AddValueTask.h"
#include "Telemetry/SetValueTask.h"
#include "Telemetry/TelemetryClient.h"

namespace traktor
{
	namespace telemetry
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.telemetry.TelemetryClient", TelemetryClient, Object)

TelemetryClient::~TelemetryClient()
{
	destroy();
}

Ref< TelemetryClient > TelemetryClient::create(const std::wstring& serverHost, const std::wstring& client)
{
	Ref< TelemetryClient > tc = new TelemetryClient();
	tc->m_serverHost = serverHost;
	tc->m_client = client;

	tc->m_thread = ThreadManager::getInstance().create(
		makeFunctor(tc.ptr(), &TelemetryClient::threadProcessQueue),
		L"Telemetry"
	);
	if (!tc->m_thread)
		return 0;

	if (!tc->m_thread->start())
		return 0;

	return tc;
}

void TelemetryClient::destroy()
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
}

void TelemetryClient::set(const std::wstring& symbol, int32_t value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_queue.push_back(new SetValueTask(m_serverHost, m_client, symbol, value));
	m_queueSignal.set();
}

void TelemetryClient::add(const std::wstring& symbol, int32_t value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_queue.push_back(new AddValueTask(m_serverHost, m_client, symbol, value));
	m_queueSignal.set();
}

void TelemetryClient::flush()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (RefArray< ITask >::iterator i = m_queue.begin(); i != m_queue.end(); ++i)
		(*i)->execute();
	m_queue.clear();
	m_queueSignal.reset();
}

TelemetryClient::TelemetryClient()
:	m_thread(0)
{
}

void TelemetryClient::threadProcessQueue()
{
	Ref< ITask > task;
	while (!m_thread->stopped())
	{
		// Wait for task to be queued.
		if (!m_queueSignal.wait(200))
			continue;

		// Get task from queue.
		m_lock.wait();
		T_FATAL_ASSERT (!m_queue.empty());
		task = m_queue.front();
		m_lock.release();

		// Execute task.
		if (task->execute())
		{
			// Successful, discard from queue.
			m_lock.wait();
			T_FATAL_ASSERT (!m_queue.empty());
			m_queue.pop_front();
			if (m_queue.empty())
				m_queueSignal.reset();
			m_lock.release();
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
