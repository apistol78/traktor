/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/SpinLock.h"
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
		static JobHeap* s_instance = nullptr;
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
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_allocatorLock);
		void* ptr = m_blockAllocator.alloc();
		if (!ptr)
			T_FATAL_ERROR;
		m_count++;
		return ptr;
	}

	void free(void* ptr)
	{
		if (!ptr)
			return;

		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_allocatorLock);
		bool result = m_blockAllocator.free(ptr);
        T_ASSERT_M (result, L"Invalid pointer"); (void)result;
		m_count--;
	}

protected:
	virtual void destroy() { delete this; }

private:
	static constexpr size_t MaxJobCount = 512 * 1024;
	static constexpr size_t MaxJobSize = sizeof(Job);

	void* m_block;
	BlockAllocator m_blockAllocator;
	SpinLock m_allocatorLock;
	int32_t m_count;

	JobHeap()
	:	m_block(Alloc::acquireAlign(MaxJobCount * MaxJobSize, 16, T_FILE_LINE))
	,	m_blockAllocator(m_block, MaxJobCount, MaxJobSize)
	,	m_count(0)
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
	Thread* current = ThreadManager::getInstance().getCurrentThread();
	while (!current->stopped())
	{
		if (m_finished)
			break;
		if (!m_jobFinishedEvent.wait(timeout))
			return false;
	}
	return m_finished;
}

void Job::cancel()
{
	m_task = nullptr;
}

void Job::stop()
{
	m_finished = true;
}

bool Job::stopped() const
{
	return m_finished || ThreadManager::getInstance().getCurrentThread()->stopped();
}

void* Job::operator new (size_t size)
{
	return JobHeap::getInstance().alloc(uint32_t(size));
}

void Job::operator delete (void* ptr)
{
	JobHeap::getInstance().free(ptr);
}

Job::Job(Event& jobFinishedEvent, const std::function< void() >& task)
:	m_jobFinishedEvent(jobFinishedEvent)
,	m_task(task)
,	m_finished(false)
{
}

}
