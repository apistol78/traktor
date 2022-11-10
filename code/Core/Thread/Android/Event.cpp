/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cassert>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include "Core/Thread/Event.h"
#include "Core/Thread/Android/Utilities.h"

namespace traktor
{
	namespace
	{

const uint32_t c_broadcast = ~0UL;

struct Internal
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	uint32_t signal;
	uint32_t waiters;
};

	}

Event::Event()
{
	Internal* in = new Internal();
	pthread_mutex_init(&in->mutex, 0);
	pthread_cond_init(&in->cond, 0);
	in->signal = 0;
	in->waiters = 0;
	m_handle = in;
}

Event::~Event()
{
	Internal* in = static_cast< Internal* >(m_handle);
	pthread_cond_destroy(&in->cond);
	pthread_mutex_destroy(&in->mutex);
	delete in;
}

void Event::pulse(int count)
{
	Internal* in = static_cast< Internal* >(m_handle);
	pthread_mutex_lock(&in->mutex);

	in->signal += count;
	pthread_cond_broadcast(&in->cond);

	pthread_mutex_unlock(&in->mutex);
}

void Event::broadcast()
{
	Internal* in = static_cast< Internal* >(m_handle);
	pthread_mutex_lock(&in->mutex);

	in->signal = c_broadcast;
	pthread_cond_broadcast(&in->cond);

	pthread_mutex_unlock(&in->mutex);
}

void Event::reset()
{
	Internal* in = static_cast< Internal* >(m_handle);
	pthread_mutex_lock(&in->mutex);

	in->signal = 0;

	pthread_mutex_unlock(&in->mutex);
}

bool Event::wait(int timeout)
{
	Internal* in = static_cast< Internal* >(m_handle);
	int rc;

	rc = pthread_mutex_lock(&in->mutex);
	T_FATAL_ASSERT (rc == 0);

	if (in->signal == 0)
	{
		++in->waiters;

		if (timeout >= 0)
		{
			timespec ts;

			clock_gettime(CLOCK_REALTIME, &ts);
			addMilliSecToTimeSpec(&ts, timeout);

			while (in->signal == 0 && rc == 0)
				rc = pthread_cond_timedwait(&in->cond, &in->mutex, &ts);
		}
		else
		{
			while (in->signal == 0 && rc == 0)
				rc = pthread_cond_wait(&in->cond, &in->mutex);
		}

		--in->waiters;
	}

	if (rc == 0 && in->signal != 0)
	{
		if (in->signal != c_broadcast)
			--in->signal;
		else if (in->waiters == 0)
			in->signal = 0;
	}

	pthread_mutex_unlock(&in->mutex);
	return bool(rc == 0);
}

}
