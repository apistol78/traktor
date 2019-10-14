#include <cstring>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include "Core/Misc/TString.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/Linux/Utilities.h"
#include "Core/Functor/Functor.h"

namespace traktor
{
	namespace
	{

struct Internal
{
	pthread_t thread;
};

void* trampoline(void* data)
{
	Functor* functor = reinterpret_cast< Functor* >(data);
	(functor->operator())();
	pthread_exit(0);
	return nullptr;
}

	}

bool Thread::start(Priority priority)
{
	pthread_attr_t attr;
	sched_param param;
	int rc;

	Internal* in = new Internal();
	in->thread = 0;

	m_handle = in;

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
		(void*)m_functor
	);

	pthread_attr_destroy(&attr);

	if (rc == 0)
		pthread_setname_np(in->thread, wstombs(m_name).c_str());
	else
	{
		m_handle = nullptr;
		delete in;
	}

	return bool(rc == 0);
}

bool Thread::wait(int timeout)
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	if (!in || !in->thread)
		return true;

	void* dummy = nullptr;
	int rc;

	if (timeout > 0)
	{
		timespec ts;

		clock_gettime(CLOCK_REALTIME, &ts);
		addMilliSecToTimeSpec(&ts, timeout);

		void* retval = nullptr;
		rc = pthread_timedjoin_np(in->thread, &retval, &ts);
		if (rc != 0)
			return false;
	}
	else if (timeout == 0)
	{
		void* retval = nullptr;
		rc = pthread_tryjoin_np(in->thread, &retval);
		if (rc != 0)
			return false;
	}
	else
	{
		rc = pthread_join(
			in->thread,
			&dummy
		);
		if (rc != 0)
			return false;
	}

	in->thread = 0;
	return true;
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
	usleep(long(duration) * 1000);
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

Thread::Thread(Functor* functor, const wchar_t* const name, int hardwareCore)
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
		m_handle = in;
	}
}

Thread::~Thread()
{
	delete reinterpret_cast< Internal* >(m_handle);
}

}
