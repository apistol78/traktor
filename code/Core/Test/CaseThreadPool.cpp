/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Test/CaseThreadPool.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/ThreadPool.h"

namespace traktor::test
{
	namespace
	{

std::atomic< int32_t > g_count(0);
bool g_errorBuilder = false;

void threadWorker()
{
	Thread* current = ThreadManager::getInstance().getCurrentThread();
	g_count++;
	current->sleep(10);
}

void threadBuilder()
{
	Thread* current = ThreadManager::getInstance().getCurrentThread();
	Thread* threads[4] = { 0 };

	for (int32_t count = 0; count < 100; ++count)
	{
		g_count = 0;

		for (int32_t i = 0; i < 4; ++i)
		{
			ThreadPool::getInstance().spawn(
				[](){ threadWorker(); },
				threads[i]
			);
		} 

		current->sleep(10);

		for (int32_t i = 0; i < 4; ++i)
			ThreadPool::getInstance().join(threads[i]);

		g_errorBuilder |= (g_count != 4);
	}
}

void threadPreview()
{
	Thread* current = ThreadManager::getInstance().getCurrentThread();
	while (!current->stopped())
		current->sleep(100);
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseThreadPool", 0, CaseThreadPool, Case)

void CaseThreadPool::run()
{
	{
		Thread* threads[16] = { 0 };

		// Spawn 16 worker threads from pool.
		for (int32_t i = 0; i < 16; ++i)
		{
			bool result = ThreadPool::getInstance().spawn(
				[](){ threadWorker(); },
				threads[i]
			);
			CASE_ASSERT(result);
			if (!result)
				return;
		}

		// Join all worker threads.
		for (int32_t i = 0; i < 16; ++i)
		{
			bool result = ThreadPool::getInstance().join(threads[i]);
			CASE_ASSERT(result);
		}

		CASE_ASSERT((int32_t)g_count == 16);
	}

	{
		// Create "builder" thread.
		Thread* threadB = ThreadManager::getInstance().create([](){ threadBuilder(); });
		threadB->start();

		// Create "preview" thread.
		Thread* threadP = nullptr;
		ThreadPool::getInstance().spawn([](){ threadPreview(); }, threadP);
		CASE_ASSERT(threadP != nullptr);

		ThreadManager::getInstance().getCurrentThread()->sleep(200);

		// Stop "preview" thread.
		ThreadPool::getInstance().stop(threadP);

		// Stop "builder" thread.
		threadB->stop();
		ThreadManager::getInstance().destroy(threadB);

		CASE_ASSERT(g_errorBuilder == false);
	}
}

}
