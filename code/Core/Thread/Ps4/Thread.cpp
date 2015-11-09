#include <kernel.h>
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
	ScePthread thread;
	ScePthreadMutex mutex;
	ScePthreadCond cond;
	Functor* functor;
	bool finished;
};

void* threadEntry(void* data)
{
	Internal* in = reinterpret_cast< Internal* >(data);

	(in->functor->operator())();

	scePthreadMutexLock(&in->mutex);
	in->finished = true;
	scePthreadCondBroadcast(&in->cond);
	scePthreadMutexUnlock(&in->mutex);

	scePthreadExit(0);
	return 0;
}
	
	}

bool Thread::start(Priority priority)
{
	int res;

	Internal* in = new Internal();
	in->functor = m_functor.ptr();
	in->finished = false;

	ScePthreadMutexattr attr;
	scePthreadMutexattrInit(&attr);
	res = scePthreadMutexInit(&in->mutex, &attr, 0);
	if (res != 0)
	{
		log::error << L"Unable to create thread mutex" << Endl;
		delete in;
		return false;
	}

	ScePthreadCondattr attr2;
	scePthreadCondattrInit(&attr2);
	res = scePthreadCondInit(&in->cond, &attr2, 0);
	if (res != 0)
	{
		log::error << L"Unable to create thread condition variable" << Endl;
		delete in;
		return false;
	}

	ScePthreadAttr attr3;
	scePthreadAttrInit(&attr3);
	scePthreadAttrSetdetachstate(&attr3, SCE_PTHREAD_CREATE_JOINABLE);
	res = scePthreadCreate(
		&in->thread,
		&attr3,
		threadEntry,
		in,
		0
	);
	if (res != 0)
	{
		log::error << L"Unable to create thread" << Endl;
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
			rc = scePthreadMutexLock(&in->mutex);
			if (rc != 0)
				return false;

			for (rc = 0; rc == 0 && !in->finished; )
			{
				rc = scePthreadCondTimedwait(&in->cond, &in->mutex, timeout * 1000);
				if (rc == SCE_KERNEL_ERROR_ETIMEDOUT)
					break;
			}

			scePthreadMutexUnlock(&in->mutex);
		}

		if (!in->finished)
			return false;
	}

	rc = scePthreadJoin(in->thread, 0);
	if (rc == 0)
		in->thread = 0;

	return bool(rc == 0);
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
	sceKernelUsleep(duration * 1000);
}

void Thread::yield()
{
	scePthreadYield();
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

	return scePthreadSelf() == in->thread;
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

		scePthreadCondDestroy(&in->cond);
		scePthreadMutexDestroy(&in->mutex);

		delete in;
	}
}

}
