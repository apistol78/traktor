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
#include "Core/Thread/Mutex.h"
#include "Core/Thread/Android/Utilities.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace
	{

struct InternalData
{
	pthread_mutex_t outer;
};

	}

Mutex::Mutex()
:	m_handle(0)
,	m_existing(false)
{
	InternalData* data = new InternalData();
	std::memset(data, 0, sizeof(InternalData));

	pthread_mutexattr_t ma;
	pthread_mutexattr_init(&ma);
	pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);

#if defined(_DEBUG)
	int rc =
#endif
	pthread_mutex_init(&data->outer, &ma);
#if defined(_DEBUG)
	T_ASSERT (rc == 0);
#endif

	m_handle = data;
}

Mutex::Mutex(const Guid& id)
:	m_handle(0)
,	m_existing(false)
{
	// @fixme Currently we just create an unnamed local mutex as
	// pthreads doesn't seem to support system wide mutexes.

	InternalData* data = new InternalData();
	std::memset(data, 0, sizeof(InternalData));

	pthread_mutexattr_t ma;
	pthread_mutexattr_init(&ma);
	pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);

#if defined(_DEBUG)
	int rc =
#endif
	pthread_mutex_init(&data->outer, &ma);
#if defined(_DEBUG)
	T_ASSERT (rc == 0);
#endif

	m_handle = data;
}

Mutex::~Mutex()
{
	delete reinterpret_cast< InternalData* >(m_handle);
}

bool Mutex::wait(int32_t timeout)
{
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);
	if (timeout < 0)
	{
		while (pthread_mutex_lock(&data->outer) != 0)
			;
	}
	else if (timeout == 0)
	{
		if (pthread_mutex_trylock(&data->outer) != 0)
			return false;
	}
	else
	{
		timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		addMilliSecToTimeSpec(&ts, timeout);
		if (pthread_mutex_timedlock(&data->outer, &ts) != 0)
			return false;
	}
	return true;
}

void Mutex::release()
{
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);

#if defined(_DEBUG)
	int rc =
#endif
	pthread_mutex_unlock(&data->outer);
#if defined(_DEBUG)
	T_ASSERT (rc == 0);
#endif
}

bool Mutex::existing() const
{
	return m_existing;
}

}
