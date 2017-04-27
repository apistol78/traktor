/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include "Core/Misc/TString.h"
#include "Core/Thread/Thread.h"
#include "Core/Functor/Functor.h"

namespace traktor
{
	namespace
	{

struct Internal
{
	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t signal;
#if defined(_DEBUG)
	char name[256];
#endif
	Functor* functor;
	bool finished;
};

void* trampoline(void* data)
{
	Internal* in = reinterpret_cast< Internal* >(data);

#if defined(_DEBUG)
	if (in->name)
		pthread_setname_np(in->name);
#endif

	(in->functor->operator())();
	in->finished = true;

	pthread_cond_signal(&in->signal);
	return 0;
}

	}

Thread::Thread(Functor* functor, const std::wstring& name, int hardwareCore)
:	m_handle(0)
,	m_id(0)
,	m_stopped(false)
,	m_functor(functor)
{
	Internal* in = new Internal();
#if defined(_DEBUG)
	std::strcpy(in->name, !name.empty() ? wstombs(name).c_str() : "Unnamed");
#endif
	in->functor = m_functor;
	in->finished = false;
	m_handle = in;
}

Thread::~Thread()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	delete in;
}

bool Thread::start(Priority priority)
{
	pthread_attr_t attr;
	sched_param param;
	int rc;
	
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	T_ASSERT (in);

	in->finished = false;
	
	pthread_mutex_init(&in->mutex, NULL);
	pthread_cond_init(&in->signal, NULL);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	// By default OSX/iOS allocate only 512kb for secondary threads.
	pthread_attr_setstacksize(&attr, 2 * 1024 * 1024);

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
	return bool(rc == 0);
}

bool Thread::wait(int timeout)
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);

	int status = 0;
	int rc = 0;
	
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
		0
	);
	
	return bool(rc == 0); 
}

bool Thread::stop(int timeout)
{
	m_stopped = true;
	return wait(timeout);
}

bool Thread::pause()
{
	return false;
}

bool Thread::resume()
{
	return false;
}

void Thread::sleep(int duration)
{
	usleep(useconds_t(duration) * 1000);
}

void Thread::yield()
{
	sched_yield();
}

bool Thread::stopped() const
{
	return m_stopped;
}

bool Thread::current() const
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	return bool(pthread_equal(in->thread, pthread_self()) == 1);
}

bool Thread::finished() const
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	return in->finished;
}

}
