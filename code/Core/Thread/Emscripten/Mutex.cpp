/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#if defined(__EMSCRIPTEN_PTHREADS__)
#	include <pthread.h>
#endif
#include "Core/Thread/Mutex.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace
	{

#if defined(__EMSCRIPTEN_PTHREADS__)
struct InternalData
{
	pthread_mutex_t outer;
};
#endif

	}

Mutex::Mutex()
:	m_existing(false)
,	m_handle(0)
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	InternalData* data = new InternalData();
	std::memset(data, 0, sizeof(InternalData));

	pthread_mutexattr_t ma;
	pthread_mutexattr_init(&ma);
	pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);

	int rc = pthread_mutex_init(&data->outer, &ma);
	T_ASSERT (rc == 0);

	m_handle = data;
#endif
}

Mutex::Mutex(const Guid& id)
:	m_existing(false)
,	m_handle(0)
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	// @fixme Currently we just create an unnamed local mutex as
	// pthreads doesn't seem to support system wide mutexes.

	InternalData* data = new InternalData();
	std::memset(data, 0, sizeof(InternalData));

	pthread_mutexattr_t ma;
	pthread_mutexattr_init(&ma);
	pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);

	int rc = pthread_mutex_init(&data->outer, &ma);
	T_ASSERT (rc == 0);

	m_handle = data;
#endif
}

Mutex::~Mutex()
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	delete reinterpret_cast< InternalData* >(m_handle);
#endif
}

bool Mutex::wait(int32_t timeout)
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);
	while (pthread_mutex_lock(&data->outer) != 0)
		;
#endif	
	return true;
}

void Mutex::release()
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);

	int rc = pthread_mutex_unlock(&data->outer);
	T_ASSERT(rc == 0);
#endif
}

bool Mutex::existing() const
{
	return m_existing;
}

}
