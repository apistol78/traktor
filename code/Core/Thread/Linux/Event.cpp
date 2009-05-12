#include <cassert>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include "Core/Thread/Event.h"

namespace traktor
{
	namespace
	{

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

	in->signal = std::max< uint32_t >(1, in->waiters);
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
	int rc = 0;
	
	pthread_mutex_lock(&in->mutex);

	if (in->signal == 0)
	{
		++in->waiters;
		
		if (timeout >= 0)
		{
			timeval now;
			timespec ts;
		
			gettimeofday(&now, 0);
			ts.tv_sec = now.tv_sec + timeout / 1000;
			ts.tv_nsec = (now.tv_usec + timeout % 1000) * 1000;			
			
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

	if (rc == 0 && in->signal > 0)
		--in->signal;

	pthread_mutex_unlock(&in->mutex);
	
	return bool(rc == 0);
}

}
