/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Job.h"

#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/SpinLock.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{
namespace
{

/*! Number of job blocks a thread keeps to itself. */
const uint32_t c_jobCacheSize = 32;

/*! Thread local cache of job blocks. */
struct JobCache
{
	void* blocks[c_jobCacheSize];
	uint32_t count;
};

thread_local JobCache t_jobCache = {};

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
		T_ASSERT_M(size <= MaxJobSize, L"Allocation size too big");

		JobCache& cache = t_jobCache;
		if (cache.count == 0)
		{
			T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_allocatorLock);
			while (cache.count < c_jobCacheSize)
			{
				void* block = m_blockAllocator.alloc();
				if (!block)
					break;
				cache.blocks[cache.count++] = block;
			}
		}

		if (cache.count == 0)
			T_FATAL_ERROR;

		return cache.blocks[--cache.count];
	}

	void free(void* ptr)
	{
		if (!ptr)
			return;

		JobCache& cache = t_jobCache;
		if (cache.count >= c_jobCacheSize)
		{
			const uint32_t flush = c_jobCacheSize / 2;
			{
				T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_allocatorLock);
				for (uint32_t i = 0; i < flush; ++i)
				{
					const bool result = m_blockAllocator.free(cache.blocks[i]);
					T_ASSERT_M(result, L"Invalid pointer");
					(void)result;
				}
			}
			for (uint32_t i = flush; i < cache.count; ++i)
				cache.blocks[i - flush] = cache.blocks[i];
			cache.count -= flush;
		}

		cache.blocks[cache.count++] = ptr;
	}

protected:
	virtual void destroy() { delete this; }

private:
	static constexpr size_t MaxJobCount = 512 * 1024;
	static constexpr size_t MaxJobSize = sizeof(Job);

	void* m_block;
	BlockAllocator m_blockAllocator;
	SpinLock m_allocatorLock;

	JobHeap()
		: m_block(Alloc::acquireAlign(MaxJobCount * MaxJobSize, 16, T_FILE_LINE))
		, m_blockAllocator(m_block, MaxJobCount, MaxJobSize)
	{
	}

	virtual ~JobHeap()
	{
		Alloc::freeAlign(m_block);
	}
};

}

std::atomic< int32_t > Job::waiters(0);

bool Job::wait(int32_t timeout)
{
	Thread* current = ThreadManager::getInstance().getCurrentThread();

	++waiters;

	bool timedOut = false;
	while (!current->stopped())
	{
		if (m_finished)
			break;
		if (!m_jobFinishedEvent.wait(timeout))
		{
			timedOut = true;
			break;
		}
	}

	--waiters;
	return timedOut ? false : (bool)m_finished;
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

void* Job::operator new(size_t size)
{
	return JobHeap::getInstance().alloc(uint32_t(size));
}

void Job::operator delete(void* ptr)
{
	JobHeap::getInstance().free(ptr);
}

Job::Job(Event& jobFinishedEvent, const std::function< void() >& task)
	: m_jobFinishedEvent(jobFinishedEvent)
	, m_task(task)
	, m_finished(false)
{
}

}
