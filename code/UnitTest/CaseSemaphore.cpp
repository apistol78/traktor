#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Mutex.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "UnitTest/CaseSemaphore.h"

namespace traktor
{
	namespace
	{

Semaphore g_semaphore;
volatile int32_t g_precious = 0;
int32_t g_error[3] = { 0, 0, 0 };
int32_t g_count[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void threadStress(int32_t index)
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();
	while (!thread->stopped())
	{
		//if (!g_semaphore.wait(0))
		//	continue;

		g_semaphore.wait();

		// << ATOMIC BEGIN >>

		int32_t current = g_precious;
		g_precious = 1;

		if (current != 0)
			Atomic::increment(g_error[current - 1]);

		Atomic::increment(g_count[index]);

		// Just increment semaphore.
		g_semaphore.wait();
		g_precious = 2;
		g_semaphore.release();
		g_precious = 3;
		thread->yield();
		g_precious = 0;

		// << ATOMIC END >>

		g_semaphore.release();
	}
}

	}

void CaseSemaphore::run()
{
	Thread* threads[16];

	for (int i = 0; i < 16; ++i)
	{
		threads[i] = ThreadManager::getInstance().create(makeStaticFunctor(threadStress, i));
		threads[i]->start();
	}

	ThreadManager::getInstance().getCurrentThread()->sleep(1000);

	for (int i = 0; i < 16; ++i)
	{
		threads[i]->stop();
		ThreadManager::getInstance().destroy(threads[i]);
	}

	for (int i = 0; i < 16; ++i)
		log::info << L"g_count[" << i << L"] = " << g_count[i] << Endl;

	CASE_ASSERT_EQUAL(g_error[0], 0);
	CASE_ASSERT_EQUAL(g_error[1], 0);
	CASE_ASSERT_EQUAL(g_error[2], 0);
}

}
