#include <pthread.h>
#include <sched.h>
#include "Core/Thread/Thread.h"
#include "Core/Functor/Functor.h"

namespace traktor
{
	namespace
	{

struct Internal
{
	pthread_t thread;
	Functor* functor;
	bool finished;
};

void* trampoline(void* data)
{
	Internal* in = reinterpret_cast< Internal* >(data);

	(in->functor->operator())();
	in->finished = true;

	pthread_exit(0);
	return 0;
}

	}

Thread::Thread(Functor* functor, const std::wstring& name, int hardwareCore)
:	m_handle(0)
,	m_id(0)
,	m_stopped(false)
,	m_functor(functor)
{
}

Thread::~Thread()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	if (in)
	{
		wait();
		delete in;
	}
}

bool Thread::start(Priority priority)
{
	pthread_attr_t attr;
	int rc;
	
	Internal* in = new Internal();

	in->functor = m_functor;
	in->finished = false;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	rc = pthread_create(
		&in->thread,
		&attr,
		trampoline,
		(void*)in
	);
	
	pthread_attr_destroy(&attr);

	if (rc == 0)
		m_handle = in;
	else
		delete in;
	
	return bool(rc == 0);
}


bool Thread::wait(int timeout)
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);

	if (in->finished)
		return true;

	int status;
	int rc;
	
	if (timeout)
		return false;
	
	rc = pthread_join(
		in->thread,
		(void**)&status
	);
	T_ASSERT(in->finished);
	
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
	::sleep((duration + 999) / 1000);
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
	if (!in)
		return false;

	return bool(pthread_equal(in->thread, pthread_self()) == 1);
}

bool Thread::finished() const
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	return in ? in->finished : true;
}

}
