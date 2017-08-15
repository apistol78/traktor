/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#if defined(__EMSCRIPTEN_PTHREADS__)
#	include <pthread.h>
#	include <sched.h>
#	include <unistd.h>
#	include <sys/time.h>
#endif
#include "Core/Thread/Thread.h"
#include "Core/Functor/Functor.h"

namespace traktor
{
	namespace
	{

#if defined(__EMSCRIPTEN_PTHREADS__)
struct Internal
{
	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t signal;
	Functor* functor;
	bool finished;
};

void* trampoline(void* data)
{
	Internal* in = reinterpret_cast< Internal* >(data);

	(in->functor->operator())();
	in->finished = true;

	pthread_cond_signal(&in->signal);
	pthread_exit(0);
	return 0;
}
#endif

	}

Thread::Thread(Functor* functor, const std::wstring& name, int hardwareCore)
:	m_handle(0)
,	m_id(0)
,	m_stopped(false)
,	m_functor(functor)
{
}

Thread::~Thread()
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	delete reinterpret_cast< Internal* >(m_handle);
#endif
}

bool Thread::start(Priority priority)
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	pthread_attr_t attr;
	sched_param param;
	int rc;

	Internal* in = new Internal();

	in->functor = m_functor;
	in->finished = false;

	pthread_mutex_init(&in->mutex, NULL);
	pthread_cond_init(&in->signal, NULL);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	std::memset(&param, 0, sizeof(param));
	switch (priority)
	{
	case Lowest:
		param.sched_priority = 15;
		break;

	case Below:
		param.sched_priority = 5;
		break;

	case Normal:
		param.sched_priority = 0;
		break;

	case Above:
		param.sched_priority = -6;
		break;

	case Highest:
		param.sched_priority = -16;
		break;
	}
	pthread_attr_setschedparam(&attr, &param);

	rc = pthread_create(
		&in->thread,
		&attr,
		trampoline,
		(void*)in
	);

	pthread_attr_destroy(&attr);

	if (rc == 0)
		m_handle = in;
	else
		delete in;

	return bool(rc == 0);
#else
	return true;
#endif
}

bool Thread::wait(int timeout)
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	if (!in)
		return true;

	void* dummy = 0;
	int rc;

	if (timeout >= 0)
	{
		pthread_mutex_lock(&in->mutex);

		timeval now;
		timespec ts;

		gettimeofday(&now, 0);
		ts.tv_sec = now.tv_sec + timeout / 1000;
		ts.tv_nsec = (now.tv_usec + (timeout % 1000) * 1000) * 1000;
		ts.tv_sec += ts.tv_nsec / 1000000000;
		ts.tv_nsec = ts.tv_nsec % 1000000000;

		for (rc = 0; rc == 0 && !in->finished; )
		{
			rc = pthread_cond_timedwait(
				&in->signal,
				&in->mutex,
				&ts
			);
		}

		pthread_mutex_unlock(&in->mutex);
		if (!in->finished)
			return false;
	}

	rc = pthread_join(
		in->thread,
		&dummy
	);

	return bool(rc == 0);
#else
	return true;
#endif
}

bool Thread::stop(int timeout)
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	m_stopped = true;
	return wait(timeout);
#else
	return true;
#endif
}

bool Thread::pause()
{
	return false;
}

bool Thread::resume()
{
	return true;
}

bool Thread::resume(Priority priority)
{
	return true;
}

void Thread::sleep(int duration)
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	usleep(long(duration) * 1000);
#endif
}

void Thread::yield()
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	sched_yield();
#endif
}

bool Thread::stopped() const
{
	return m_stopped;
}

bool Thread::current() const
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	if (!in)
		return false;

	return bool(pthread_equal(in->thread, pthread_self()) == 1);
#else
	return true;
#endif
}

bool Thread::finished() const
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	return in ? in->finished : true;
#else
	return true;
#endif
}

}
