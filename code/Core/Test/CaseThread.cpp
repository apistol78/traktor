#include "Core/Test/CaseThread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Atomic.h"

namespace traktor
{
	namespace test
	{
		namespace
		{

Event g_event;
int32_t g_got;

void threadEvent()
{
	if (g_event.wait(5000))
		Atomic::increment(g_got);
}

Semaphore g_semaphore;
uint32_t g_count = 0;
uint32_t g_error = 0;

void sharedAccess()
{
	g_semaphore.wait();
	for (int32_t i = 0; i < 100; ++i)
	{
		g_count++;
		if (g_count > 1)
			g_error++;
		g_count--;
	}
	g_semaphore.release();
}

void threadSemaphore()
{
	for (int32_t i = 0; i < 100000; ++i)
		sharedAccess();
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseThread", 0, CaseThread, Case)

void CaseThread::run()
{
	// Test signals.
	{
		Thread* threads[16] = { 0 };
		g_got = 0;

		// Create and start all test threads.
		for (int32_t i = 0; i < 16; ++i)
			threads[i] = ThreadManager::getInstance().create(makeStaticFunctor(threadEvent), L"Event test");
		for (int32_t i = 0; i < 16; ++i)
			threads[i]->start();

		// Wait a bit to ensure threads are waiting on event.
		ThreadManager::getInstance().getCurrentThread()->sleep(100);

		// Pulse event.
		g_event.pulse();

		// Wait a bit further to ensure thread got pulse have time to shut down.
		ThreadManager::getInstance().getCurrentThread()->sleep(100);

		// Count number of threads that has finished.
		int32_t finished = 0;
		for (int32_t i = 0; i < 16; ++i)
		{
			if (threads[i]->finished())
				finished++;
		}

		// Ensure only one thread got woken by the pulse.
		CASE_ASSERT_EQUAL(finished, 1);
		CASE_ASSERT_EQUAL(g_got, 1);

		// Broadcast event.
		g_event.broadcast();

		// Wait a bit again to ensure all threads have time to shut down.
		ThreadManager::getInstance().getCurrentThread()->sleep(100);

		// Count number of threads that has finished.
		finished = 0;
		for (int32_t i = 0; i < 16; ++i)
		{
			if (threads[i]->finished())
				finished++;
		}

		// Ensure all threads got woken and thus terminated by the broadcast.
		CASE_ASSERT_EQUAL(finished, 16);
		CASE_ASSERT_EQUAL(g_got, 16);

		// Cleanup all threads.
		for (int32_t i = 0; i < 16; ++i)
			ThreadManager::getInstance().destroy(threads[i]);
	}

	// Test semaphores.
	{
		Thread* threads[16] = { 0 };

		// Create and start all test threads.
		for (int32_t i = 0; i < 16; ++i)
			threads[i] = ThreadManager::getInstance().create(makeStaticFunctor(threadEvent), L"Event test");
		for (int32_t i = 0; i < 16; ++i)
			threads[i]->start();

		ThreadManager::getInstance().getCurrentThread()->sleep(3000);

		// Wait until all threads has finished.
		for (int32_t i = 0; i < 16; ++i)
			threads[i]->wait();

		// Cleanup all threads.
		for (int32_t i = 0; i < 16; ++i)
			ThreadManager::getInstance().destroy(threads[i]);

		CASE_ASSERT_EQUAL(g_error, 0);
	}
}

	}
}
