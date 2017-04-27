/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if defined(__EMSCRIPTEN_PTHREADS__)
#	include <pthread.h>
#	include <sys/time.h>
#	include <unistd.h>
#	include <time.h>
#endif
#include "Core/Thread/Signal.h"

namespace traktor
{
	namespace
	{

#if defined(__EMSCRIPTEN_PTHREADS__)
struct Internal
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	uint32_t signal;
};
#endif

	}

Signal::Signal()
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	Internal* in = new Internal();
	pthread_mutex_init(&in->mutex, 0);
	pthread_cond_init(&in->cond, 0);
	in->signal = 0;
	m_handle = in;
#endif
}

Signal::~Signal()
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	Internal* in = static_cast< Internal* >(m_handle);
	pthread_cond_destroy(&in->cond);
	pthread_mutex_destroy(&in->mutex);
	delete in;
#endif
}

void Signal::set()
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	Internal* in = static_cast< Internal* >(m_handle);
	pthread_mutex_lock(&in->mutex);

	in->signal = 1;
	pthread_cond_broadcast(&in->cond);

	pthread_mutex_unlock(&in->mutex);
#endif
}

void Signal::reset()
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	Internal* in = static_cast< Internal* >(m_handle);
	pthread_mutex_lock(&in->mutex);

	in->signal = 0;

	pthread_mutex_unlock(&in->mutex);
#endif
}

bool Signal::wait(int timeout)
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	Internal* in = static_cast< Internal* >(m_handle);
	int rc = 0;

	pthread_mutex_lock(&in->mutex);

	if (in->signal == 0)
	{
		if (timeout >= 0)
		{
			timeval now;
			timespec ts;

			gettimeofday(&now, 0);
			ts.tv_sec = now.tv_sec + timeout / 1000;
			ts.tv_nsec = (now.tv_usec + (timeout % 1000) * 1000) * 1000;
			ts.tv_sec += ts.tv_nsec / 1000000000;
			ts.tv_nsec = ts.tv_nsec % 1000000000;

			while (in->signal == 0 && rc == 0)
				rc = pthread_cond_timedwait(&in->cond, &in->mutex, &ts);
		}
		else
		{
			while (in->signal == 0 && rc == 0)
				rc = pthread_cond_wait(&in->cond, &in->mutex);
		}
	}

	pthread_mutex_unlock(&in->mutex);

	return bool(rc == 0);
#else
	return true;
#endif
}

}
