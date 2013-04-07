#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{
	namespace
	{

class JobHeap : public ISingleton
{
public:
	static JobHeap& getInstance()
	{
		static JobHeap* s_instance = 0;
		if (!s_instance)
		{
			s_instance = new JobHeap();
			SingletonManager::getInstance().add(s_instance);
		}
		return *s_instance;
	}

	void* alloc(uint32_t size)
	{
		T_ASSERT_M (size <= MaxJobSize, L"Allocation size too big");
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_allocatorLock);
		void* ptr = m_blockAllocator.alloc();
		if (!ptr)
			T_FATAL_ERROR;
#if defined(_DEBUG)
		m_count++;
#endif
		return ptr;
	}

	void free(void* ptr)
	{
		if (!ptr)
			return;

		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_allocatorLock);
		bool result = m_blockAllocator.free(ptr);
		T_ASSERT_M (result, L"Invalid pointer");
#if defined(_DEBUG)
		m_count--;
#endif
	}

protected:
	virtual void destroy() { delete this; }

private:
	enum { MaxJobCount = 64*1024 };
	enum { MaxJobSize = sizeof(Job) };

	void* m_block;
	BlockAllocator m_blockAllocator;
	Semaphore m_allocatorLock;
#if defined(_DEBUG)
	int32_t m_count;
#endif

	JobHeap()
	:	m_block(Alloc::acquireAlign(MaxJobCount * MaxJobSize, 16, T_FILE_LINE))
	,	m_blockAllocator(m_block, MaxJobCount, MaxJobSize)
#if defined(_DEBUG)
	,	m_count(0)
#endif
	{
	}

	virtual ~JobHeap()
	{
		T_ASSERT_M (m_count == 0, L"There are still jobs allocated, memory leak?");
		Alloc::freeAlign(m_block);
	}
};

	}

bool Job::wait(int32_t timeout)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (
		!m_finished &&
		!currentThread->stopped() &&
		m_functor != 0
	)
	{
		if (!m_jobFinishedEvent.wait(timeout >= 0 ? timeout : 100))
		{
			if (timeout >= 0)
				return m_finished;
		}
		// A job has been finished; check if it this
		// and in such case return true.
	}
	return true;
}

bool Job::stopped() const
{
	if (m_stopped || m_finished)
		return true;
	else
		return ThreadManager::getInstance().getCurrentThread()->stopped();
}

void* Job::operator new (size_t size)
{
	return JobHeap::getInstance().alloc(uint32_t(size));
}

void Job::operator delete (void* ptr)
{
	JobHeap::getInstance().free(ptr);
}

Job::Job(Functor* functor, Event& jobFinishedEvent)
:	m_functor(functor)
,	m_jobFinishedEvent(jobFinishedEvent)
,	m_finished(false)
,	m_stopped(false)
{
}

}
