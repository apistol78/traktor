/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include "Core/Misc/TString.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/OsX/Utilities.h"

namespace traktor
{
	namespace
	{

struct Internal
{
	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t signal;
	std::function< void() > fn;
	uint32_t* id;
	std::atomic< bool > finished;
};

void* trampoline(void* data)
{
	Internal* in = reinterpret_cast< Internal* >(data);
	*in->id = (uint32_t)(intptr_t)in->thread;
	in->fn();
	in->finished = true;
	pthread_cond_signal(&in->signal);
	return 0;
}

	}

bool Thread::start(Priority priority)
{
	pthread_attr_t attr;
	sched_param param;
	int rc;

	Internal* in = new Internal();
	in->thread = 0;
	in->fn = m_fn;
	in->id = &m_id;
	in->finished = false;

	m_handle = in;

	pthread_mutex_init(&in->mutex, NULL);
	pthread_cond_init(&in->signal, NULL);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize(&attr, 2 * 1024 * 1024);

	std::memset(&param, 0, sizeof(param));
	switch (priority)
	{
	case Lowest:
		param.sched_priority = -15;
		break;

	case Below:
		param.sched_priority = -5;
		break;

	case Normal:
		param.sched_priority = 0;
		break;

	case Above:
		param.sched_priority = 20;
		break;

	case Highest:
		param.sched_priority = 45;
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
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

	if (rc != 0)
	{
		m_handle = nullptr;
		delete in;
	}

	return bool(rc == 0);
}

bool Thread::wait(int timeout)
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);

	if (in->finished)
		return true;

	if (timeout >= 0)
	{
		timespec ts;

		clock_gettime(CLOCK_REALTIME, &ts);
		addMilliSecToTimeSpec(&ts, timeout);

		pthread_mutex_lock(&in->mutex);
		for (int rc = 0; rc == 0 && in->finished == false; )
		{
			rc = pthread_cond_timedwait(
				&in->signal,
				&in->mutex,
				&ts
			);
		}
		bool finished = in->finished;
		pthread_mutex_unlock(&in->mutex);

		if (!finished)
			return false;
	}

	int rc = pthread_join(
		in->thread,
		nullptr
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
	m_stopped = false;
	return true;
}

bool Thread::resume(Priority priority)
{
	m_stopped = false;
	return true;
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
	if (in && in->thread != 0)
		return bool(pthread_equal(in->thread, pthread_self()) != 0);
	else
		return false;
}

bool Thread::finished() const
{
	return const_cast< Thread* >(this)->wait(0);
}

Thread::Thread(const std::function< void() >& fn, const wchar_t* const name, int32_t hardwareCore)
:	m_fn(fn)
,	m_name(name)
,	m_hardwareCore(hardwareCore)
{
	if (!fn)
	{
		// Assume is main thread, only main thread is allowed to pass null as functor.
		Internal* in = new Internal();
		in->thread = pthread_self();
		m_handle = in;
	}
}

Thread::~Thread()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	delete in;
}

}
