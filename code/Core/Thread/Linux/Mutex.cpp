/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Core/Thread/Mutex.h"
#include "Core/Thread/Linux/Utilities.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace
	{

struct InternalData
{
	pthread_mutex_t nonshared;
	pthread_mutex_t* outer;
	int shmid;
	Guid id;
};

	}

Mutex::Mutex()
:	m_existing(false)
,	m_handle(nullptr)
{
	InternalData* data = new InternalData();
	std::memset(data, 0, sizeof(InternalData));

	data->outer = &data->nonshared;

	pthread_mutexattr_t ma;
	pthread_mutexattr_init(&ma);
	pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);

	int rc = pthread_mutex_init(data->outer, &ma);
	T_ASSERT (rc == 0);

	m_handle = data;
}

Mutex::Mutex(const Guid& id)
:	m_existing(false)
,	m_handle(nullptr)
{
	InternalData* data = new InternalData();
	std::memset(data, 0, sizeof(InternalData));

	data->shmid = shm_open(wstombs(id.format()).c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	const int result = ftruncate(data->shmid, sizeof(pthread_mutex_t));
	(void)result;

	data->outer = (pthread_mutex_t*)mmap(nullptr, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, data->shmid, 0);
	data->id = id;

	pthread_mutexattr_t ma;
	pthread_mutexattr_init(&ma);
	pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);

	int rc = pthread_mutex_init(data->outer, &ma);
	T_ASSERT (rc == 0);

	m_handle = data;
}

Mutex::~Mutex()
{
	InternalData* data = new InternalData();
	if (data->outer != nullptr && data->outer != &data->nonshared)
	{
		munmap(data->outer, sizeof(pthread_mutex_t));
		shm_unlink(wstombs(data->id.format()).c_str());
	}
	delete data;
}

bool Mutex::wait(int32_t timeout)
{
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);
	if (timeout < 0)
	{
		while (pthread_mutex_lock(data->outer) != 0)
			;
	}
	else if (timeout == 0)
	{
		if (pthread_mutex_trylock(data->outer) != 0)
			return false;
	}	
	else
	{
		timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		addMilliSecToTimeSpec(&ts, timeout);
		if (pthread_mutex_timedlock(data->outer, &ts) != 0)
			return false;
	}
	return true;
}

void Mutex::release()
{
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);
	int rc = pthread_mutex_unlock(data->outer);
	T_ASSERT(rc == 0);
}

bool Mutex::existing() const
{
	return m_existing;
}

}
