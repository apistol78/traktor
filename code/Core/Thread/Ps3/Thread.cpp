#include <sys/ppu_thread.h>
#include <sys/synchronization.h>
#include <sys/process.h>
#include <sys/timer.h>
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Thread.h"

namespace traktor
{
	namespace
	{

struct Internal
{
	sys_ppu_thread_t thread;
	sys_mutex_t mutex;
	sys_cond_t cond;
	Functor* functor;
	bool finished;
};

void threadEntry(uint64_t data)
{
	Internal* in = reinterpret_cast< Internal* >(data);

	(in->functor->operator())();

	sys_mutex_lock(in->mutex, SYS_NO_TIMEOUT);
	in->finished = true;
	sys_cond_signal_all(in->cond);
	sys_mutex_unlock(in->mutex);

	sys_ppu_thread_exit(0);
}
	
	}

bool Thread::start(Priority priority)
{
	const uint32_t c_stackSize = 256 * 1024;
	int res;

	Internal* in = new Internal();
	in->functor = m_functor.ptr();
	in->finished = false;

	sys_mutex_attribute_t attr;
	sys_mutex_attribute_initialize(attr);

	res = sys_mutex_create(&in->mutex, &attr);
	if (res != CELL_OK)
	{
		log::error << L"Unable to create ppu thread mutex" << Endl;
		delete in;
		return false;
	}

	sys_cond_attribute_t attr2;
	sys_cond_attribute_initialize(attr2);

	res = sys_cond_create(&in->cond, in->mutex, &attr2);
	if (res != CELL_OK)
	{
		log::error << L"Unable to create ppu thread condition variable" << Endl;
		delete in;
		return false;
	}

	res = sys_ppu_thread_create(
		&in->thread,
		threadEntry,
		(uint64_t)in,
		1535 - int(priority) * 767,
		c_stackSize,
		SYS_PPU_THREAD_CREATE_JOINABLE,
		m_name.c_str()
	);
	if (res != CELL_OK)
	{
		log::error << L"Unable to create ppu thread" << Endl;
		switch (res)
		{
		case ENOMEM:
			log::error << L"ENOMEM" << Endl;
			break;
		case EAGAIN:
			log::error << L"EAGAIN" << Endl;
			break;
		case EINVAL:
			log::error << L"EINVAL" << Endl;
			break;
		case EFAULT:
			log::error << L"EFAULT" << Endl;
			break;
		case EPERM:
			log::error << L"EPERM" << Endl;
			break;
		default:
			log::error << L"Unknown reason" << Endl;
			break;
		}
		delete in;
		return false;
	}

	m_handle = in;
	return true;
}

bool Thread::wait(int timeout)
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	T_ASSERT (in);

	if (!in->thread)
		return true;

	int rc;

	if (timeout >= 0)
	{
		if (timeout > 0)
		{
			rc = sys_mutex_lock(in->mutex, SYS_NO_TIMEOUT);
			if (rc != CELL_OK)
				return false;

			for (rc = 0; rc == 0 && !in->finished; )
			{
				rc = sys_cond_wait(in->cond, usecond_t(timeout) * 1000);
				if (rc == ETIMEDOUT)
					break;
			}

			sys_mutex_unlock(in->mutex);
		}

		if (!in->finished)
			return false;
	}

	uint64_t status;
	rc = sys_ppu_thread_join(in->thread, &status);

	if (rc == CELL_OK)
		in->thread = 0;

	return bool(rc == CELL_OK);
}

bool Thread::stop(int timeout)
{
	m_stopped = true;
	return wait(timeout);
}

bool Thread::pause()
{
	T_ASSERT (0);
	return false;
}

bool Thread::resume()
{
	T_ASSERT (0);
	return false;
}

void Thread::sleep(int duration)
{
	sys_timer_usleep(usecond_t(duration) * 1000);
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
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	if (!in)
		return false;

	sys_ppu_thread_t current;
	sys_ppu_thread_get_id(&current);

	return bool(current == in->thread);
}

bool Thread::finished() const
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	T_ASSERT (in);
	return in->finished;
}

Thread::Thread(Functor* functor, const std::wstring& name, int hardwareCore)
:	m_handle(0)
,	m_id(0)
,	m_stopped(false)
,	m_functor(functor)
,	m_name(wstombs(name))
,	m_hardwareCore(hardwareCore)
{
}

Thread::~Thread()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	if (in)
	{
		if (!m_stopped)
			stop();

		sys_cond_destroy(in->cond);
		sys_mutex_destroy(in->mutex);

		delete in;
	}
}

}
