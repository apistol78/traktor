#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Online/Impl/TaskQueue.h"
#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskQueue", TaskQueue, Object)

bool TaskQueue::create(ITask* idleTask)
{
	m_idleTask = idleTask;

	m_thread = ThreadManager::getInstance().create(makeFunctor(this, &TaskQueue::threadQueue), L"Online task queue");
	if (!m_thread || !m_thread->start())
		return false;

	return true;
}

void TaskQueue::destroy()
{
	flush();

	if (m_thread)
	{
		m_thread->stop();
		ThreadManager::getInstance().destroy(m_thread);
		m_thread = 0;
	}

	m_idleTask = 0;
}

bool TaskQueue::add(ITask* task)
{
	T_ASSERT (task);

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_queueLock);
	m_queue.push_back(task);
	m_queuedSignal.set();

	return true;
}

void TaskQueue::flush()
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	for (bool queueEmpty = false; !queueEmpty;)
	{
		m_queueLock.wait();
		queueEmpty = m_queue.empty();
		m_queueLock.release();
		if (!queueEmpty)
			currentThread->sleep(100);
	}
}

void TaskQueue::threadQueue()
{
	while (!m_thread->stopped())
	{
		if (!m_queuedSignal.wait(1000 / 60))
		{
			if (m_idleTask)
				m_idleTask->execute(this);
			continue;
		}

		if (!m_queueLock.wait(100))
			continue;

		if (m_queue.empty())
		{
			// No more queued tasks.
			m_queuedSignal.reset();
			m_queueLock.release();
			continue;
		}

		Ref< ITask > task = m_queue.front();
		m_queue.pop_front();

		m_queueLock.release();

		if (task)
			task->execute(this);
	}
}

	}
}
