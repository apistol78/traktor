#include "Core/Test/CaseThread.h"
#include "Core/Thread/Signal.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/Event.h"

namespace traktor::test
{
	namespace
	{

Signal g_signal1;
Signal g_signal2;
Event g_event;
std::atomic< int32_t > g_got;

void threadSignal()
{
	g_signal1.wait();
	g_got++;
	g_signal2.wait();
	g_got++;
	g_signal1.wait();
	g_got++;
}

void threadEvent()
{
	if (g_event.wait(5000))
		g_got++;
}

Semaphore g_semaphore;
uint32_t g_error = 0;

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseThread", 0, CaseThread, Case)

void CaseThread::run()
{
	// Test signals.
	{
		Thread* threads[16] = { 0 };
		g_got = 0;

		g_signal1.set();

		// Create and start all test threads.
		for (int32_t i = 0; i < 16; ++i)
			threads[i] = ThreadManager::getInstance().create([](){ threadSignal(); }, L"Signal test");
		for (int32_t i = 0; i < 16; ++i)
			threads[i]->start();

		ThreadManager::getInstance().getCurrentThread()->sleep(200);

		g_signal1.reset();

		CASE_ASSERT_EQUAL((int32_t)g_got, 16);

		g_signal2.set();
		ThreadManager::getInstance().getCurrentThread()->sleep(200);

		CASE_ASSERT_EQUAL((int32_t)g_got, 32);

		g_signal1.set();
		ThreadManager::getInstance().getCurrentThread()->sleep(200);

		CASE_ASSERT_EQUAL((int32_t)g_got, 48);

		int32_t finished = 0;
		for (int32_t i = 0; i < 16; ++i)
		{
			if (threads[i]->finished())
				finished++;
		}

		CASE_ASSERT_EQUAL(finished, 16);

		// Cleanup all threads.
		for (int32_t i = 0; i < 16; ++i)
			ThreadManager::getInstance().destroy(threads[i]);		
	}

	// Test events.
	{
		Thread* threads[16] = { 0 };
		g_got = 0;

		// Create and start all test threads.
		for (int32_t i = 0; i < 16; ++i)
			threads[i] = ThreadManager::getInstance().create([](){ threadEvent(); }, L"Event test");
		for (int32_t i = 0; i < 16; ++i)
			threads[i]->start();

		// Wait a bit to ensure threads are waiting on event.
		ThreadManager::getInstance().getCurrentThread()->sleep(100);

		// Pulse event.
		g_event.pulse(2);

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
		CASE_ASSERT_EQUAL(finished, 2);
		CASE_ASSERT_EQUAL((int32_t)g_got, 2);

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
		CASE_ASSERT_EQUAL((int32_t)g_got, 16);

		// Cleanup all threads.
		for (int32_t i = 0; i < 16; ++i)
			ThreadManager::getInstance().destroy(threads[i]);
	}

	// Test semaphores.
	{
		Thread* threads[16] = { 0 };

		// Create and start all test threads.
		for (int32_t i = 0; i < 16; ++i)
			threads[i] = ThreadManager::getInstance().create([](){ threadEvent(); }, L"Event test");
		for (int32_t i = 0; i < 16; ++i)
			threads[i]->start();

		ThreadManager::getInstance().getCurrentThread()->sleep(1000);

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
