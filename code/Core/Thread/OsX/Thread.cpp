#include <cstring>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include "Core/Misc/TString.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/OsX/Utilities.h"
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
	Functor* functor;
	int32_t finished;
};

void* trampoline(void* data)
{
	Internal* in = reinterpret_cast< Internal* >(data);

	(in->functor->operator())();
	Atomic::exchange(in->finished, 1);

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
	in->functor = m_functor;
	in->finished = 0;

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
		for (int rc = 0; rc == 0 && in->finished == 0; )
		{
			rc = pthread_cond_timedwait(
				&in->signal,
				&in->mutex,
				&ts
			);
		}
		bool finished = (bool)(in->finished != 0);
		pthread_mutex_unlock(&in->mutex);

		if (!finished)
			return false;
	}

	int rc = pthread_join(
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

Thread::Thread(Functor* functor, const wchar_t* const name, int32_t hardwareCore)
:	m_handle(nullptr)
,	m_id(0)
,	m_stopped(false)
,	m_functor(functor)
,	m_name(name)
,	m_hardwareCore(hardwareCore)
{
	if (!functor)
	{
		// Assume is main thread, only main thread is allowed to pass null as functor.
		Internal* in = new Internal();
		in->thread = pthread_self();
		in->functor = nullptr;
		m_handle = in;
	}
}

Thread::~Thread()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	delete in;
}

}
