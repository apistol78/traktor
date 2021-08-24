#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Test/CaseSemaphore.h"
#include "Core/Timer/Timer.h"

namespace traktor
{
	namespace test
	{
		namespace
		{

Semaphore g_semaphore;
int32_t g_count[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int32_t g_concurrent = 0;
bool g_errorNullThread = false;
bool g_errorConcurrent = false;
bool g_errorStopped = false;

void threadStress(int32_t index)
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();
	if (thread == nullptr)
	{
		g_errorNullThread = true;
		return;
	}

	Atomic::increment(g_count[index]);

	Timer timer;
	while (!thread->stopped() && timer.getElapsedTime() < 4000.0)
	{
		g_semaphore.wait();
		Atomic::increment(g_concurrent);
		
		thread->yield();

		if (g_concurrent > 1)
			g_errorConcurrent = true;

		Atomic::decrement(g_concurrent);
		g_semaphore.release();
	}

	if (!thread->stopped())
		g_errorStopped = true;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseSemaphore", 0, CaseSemaphore, Case)

void CaseSemaphore::run()
{
	Thread* threads[16] = { nullptr };

	// Start 16 threads.
	for (int i = 0; i < 16; ++i)
	{
		threads[i] = ThreadManager::getInstance().create([=](){ threadStress(i); }, L"Semaphore test");
		threads[i]->start();
	}

	// Let threads execute 1 second.
	ThreadManager::getInstance().getCurrentThread()->sleep(1000);

	// Stop all threads.
	for (int i = 0; i < 16; ++i)
	{
		threads[i]->stop();
		CASE_ASSERT_EQUAL(threads[i]->stopped(), true);
		ThreadManager::getInstance().destroy(threads[i]);
	}

	// Ensure index argument work.
	for (int i = 0; i < 16; ++i)
	{
		CASE_ASSERT_EQUAL(g_count[i], 1);
	}

	// Concurrent counter should not offset.
	CASE_ASSERT_EQUAL(g_concurrent, 0);

	// Error check.
	CASE_ASSERT_EQUAL(g_errorNullThread, false);
	CASE_ASSERT_EQUAL(g_errorConcurrent, false);
	CASE_ASSERT_EQUAL(g_errorStopped, false);
}

	}
}
