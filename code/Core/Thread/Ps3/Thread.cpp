#include <cassert>
#include <sys/ppu_thread.h>
#include <sys/process.h>
#include <sys/timer.h>
#include "Core/Thread/Thread.h"
#include "Core/Functor/Functor.h"

namespace traktor
{
	namespace
	{
	
void threadEntry(uint64_t data)
{
	Functor* functor = reinterpret_cast< Functor* >(data);
	assert (functor);
	
	(functor->operator())();
	
	sys_ppu_thread_exit(0);
}
	
	}

Thread::Thread(Functor* functor)
:	m_handle(0)
,	m_id(0)
,	m_stopped(false)
,	m_functor(functor)
{
}

Thread::~Thread()
{
	sys_ppu_thread_t* thread = reinterpret_cast< sys_ppu_thread_t* >(m_handle);
	delete thread;
}

bool Thread::start(Priority priority)
{
	const uint32_t c_priorities[] = { 0, 2000, 4000 };
	const uint32_t c_stackSize = 4096;

	sys_ppu_thread_t* thread = new sys_ppu_thread_t();

	int rc = sys_ppu_thread_create(
		thread,
		threadEntry,
		(uint64_t)m_functor,
		c_priorities[priority],
		c_stackSize,
		SYS_PPU_THREAD_CREATE_JOINABLE,
		"Thread"
	);

	m_handle = thread;

	return bool(rc == CELL_OK);
}

bool Thread::wait(int timeout)
{
	sys_ppu_thread_t* thread = reinterpret_cast< sys_ppu_thread_t* >(m_handle);
	assert (thread);

	assert (timeout == 0);

	uint64_t status;
	int rc = sys_ppu_thread_join(*thread, &status);

	return bool(rc == CELL_OK);
}

bool Thread::stop(int timeout)
{
	m_stopped = true;
	return wait(timeout);
}

bool Thread::pause()
{
	assert (0);
	return false;
}

bool Thread::resume()
{
	assert (0);
	return false;
}

void Thread::sleep(int duration)
{
	sys_timer_usleep(duration);
}

void Thread::yield()
{
	sys_ppu_thread_yield();
}

bool Thread::stopped() const
{
	return m_stopped;
}

bool Thread::current() const
{
	sys_ppu_thread_t* thread = reinterpret_cast< sys_ppu_thread_t* >(m_handle);
	if (!thread)
		return false;

	sys_ppu_thread_t current;
	sys_ppu_thread_get_id(&current);

	return bool(current == *thread);
}

bool Thread::finished() const
{
	return false;
}

}
