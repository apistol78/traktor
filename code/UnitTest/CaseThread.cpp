/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "UnitTest/CaseThread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Atomic.h"

namespace traktor
{
	namespace
	{

Event g_event;
uint32_t g_got;

void thread_1()
{
	if (g_event.wait(5000))
		g_got++;
}

void thread_2()
{
	if (g_event.wait(5000))
		g_got++;
}

void thread_3()
{
	if (g_event.wait(5000))
		g_got++;
}

Semaphore g_semaphore;
uint32_t g_count = 0;
uint32_t g_error = 0;

void shared_access()
{
	g_semaphore.wait();
	for (int i = 0; i < 100; ++i)
	{
		g_count++;
		if (g_count > 1)
			g_error++;
		g_count--;
	}
	g_semaphore.release();
}

void sem_thread_1()
{
	for (int i = 0; i < 100000; ++i)
		shared_access();
}

void sem_thread_2()
{
	for (int i = 0; i < 100000; ++i)
		shared_access();
}

void sem_thread_3()
{
	for (int i = 0; i < 100000; ++i)
		shared_access();
}

	}

void CaseThread::run()
{
	// Ensure atomics work.
	int32_t value = 0;
	Atomic::increment(value);
	CASE_ASSERT_EQUAL(value, 1);
	Atomic::decrement(value);
	CASE_ASSERT_EQUAL(value, 0);

	// Test signals.
	{
		g_got = 0;

		Thread* thread1 = ThreadManager::getInstance().create(makeStaticFunctor(thread_1), L"Thread 1");
		Thread* thread2 = ThreadManager::getInstance().create(makeStaticFunctor(thread_2), L"Thread 2");
		Thread* thread3 = ThreadManager::getInstance().create(makeStaticFunctor(thread_3), L"Thread 3");

		thread1->start();
		thread2->start();
		thread3->start();

		ThreadManager::getInstance().getCurrentThread()->sleep(100);

		g_event.pulse();

		ThreadManager::getInstance().getCurrentThread()->sleep(100);

		uint32_t finished = 0;
		if (thread1->finished())
			finished++;
		if (thread2->finished())
			finished++;
		if (thread3->finished())
			finished++;

		// Ensure only one thread got woken by the pulse.
		CASE_ASSERT_EQUAL(finished, 1);
		CASE_ASSERT_EQUAL(g_got, 1);

		g_event.broadcast();

		ThreadManager::getInstance().getCurrentThread()->sleep(100);

		finished = 0;
		if (thread1->finished())
			finished++;
		if (thread2->finished())
			finished++;
		if (thread3->finished())
			finished++;

		// Ensure all threads got woken and thus terminated by the broadcast.
		CASE_ASSERT_EQUAL(finished, 3);
		CASE_ASSERT_EQUAL(g_got, 3);

		ThreadManager::getInstance().destroy(thread3);
		ThreadManager::getInstance().destroy(thread2);
		ThreadManager::getInstance().destroy(thread1);
	}

	// Test semaphores.
	{
		Thread* thread1 = ThreadManager::getInstance().create(makeStaticFunctor(sem_thread_1), L"Thread 1");
		Thread* thread2 = ThreadManager::getInstance().create(makeStaticFunctor(sem_thread_2), L"Thread 2");
		Thread* thread3 = ThreadManager::getInstance().create(makeStaticFunctor(sem_thread_3), L"Thread 3");

		thread1->start();
		thread2->start();
		thread3->start();

		ThreadManager::getInstance().getCurrentThread()->sleep(3000);

		thread1->wait();
		thread2->wait();
		thread3->wait();

		ThreadManager::getInstance().destroy(thread3);
		ThreadManager::getInstance().destroy(thread2);
		ThreadManager::getInstance().destroy(thread1);

		CASE_ASSERT_EQUAL(g_error, 0);
	}
}

}
