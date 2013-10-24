#include "Core/Thread/Thread.h"
#include "Core/Functor/Functor.h"

namespace traktor
{

Thread::Thread(Functor* functor, const std::wstring& name, int hardwareCore)
:	m_handle(0)
,	m_id(0)
,	m_stopped(false)
,	m_functor(functor)
{
}

Thread::~Thread()
{
}

bool Thread::start(Priority priority)
{
	return false;
}

bool Thread::wait(int timeout)
{
	return true;
}

bool Thread::stop(int timeout)
{
	m_stopped = true;
	return true;
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
}

void Thread::yield()
{
}

bool Thread::stopped() const
{
	return m_stopped;
}

bool Thread::current() const
{
	return true;
}

bool Thread::finished() const
{
	return true;
}

}
