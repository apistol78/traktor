#include <process.h>
#include "Core/System.h"
#include "Core/Thread/Thread.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace
	{

struct THREADNAME_INFO
{
	DWORD dwType;
	LPCSTR szName;
	DWORD dwThreadID;
	DWORD dwFlags;
};

unsigned __stdcall threadProc(void* lparam)
{
	std::function< void() >* pfn = reinterpret_cast< std::function< void() >* >(lparam);
	(*pfn)();
	return 0;
}

	}

Thread::Thread(const std::function< void() >& fn, const wchar_t* const name, int32_t hardwareCore)
:	m_fn(fn)
,	m_name(name)
,	m_hardwareCore(hardwareCore)
{
}

Thread::~Thread()
{
}

bool Thread::start(Priority priority)
{
	if (m_handle)
		return false;

	m_handle = (void*)_beginthreadex(
		0,
		0,
		&threadProc,
		(void*)&m_fn,
		CREATE_SUSPENDED,
		&m_id
	);
	if (!m_handle)
		return false;

	SetThreadDescription(m_handle, m_name);
	return resume(priority);
}

bool Thread::wait(int32_t timeout)
{
	return bool(WaitForSingleObject(m_handle, (timeout < 0) ? INFINITE : timeout) == WAIT_OBJECT_0);
}

bool Thread::stop(int32_t timeout)
{
	m_stopped = true;
	return wait(timeout);
}

bool Thread::pause()
{
	return bool(SuspendThread(m_handle) != ~0UL);
}

bool Thread::resume()
{
	m_stopped = false;
	return bool(ResumeThread(m_handle) != ~0UL);
}

bool Thread::resume(Priority priority)
{
	const int c_priorities[] =
	{
		THREAD_PRIORITY_LOWEST,
		THREAD_PRIORITY_BELOW_NORMAL,
		THREAD_PRIORITY_NORMAL,
		THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_HIGHEST
	};
	SetThreadPriority(m_handle, c_priorities[int(priority) + 2]);
	return resume();
}

void Thread::sleep(int duration)
{
	MMRESULT result = TIMERR_NOCANDO;
	if (duration <= 10)
		result = timeBeginPeriod(1);

	Sleep(duration);

	if (result == TIMERR_NOERROR)
		timeEndPeriod(1);
}

void Thread::yield()
{
	Sleep(0);
}

bool Thread::stopped() const
{
	return m_stopped;
}

bool Thread::current() const
{
	return bool(GetCurrentThreadId() == m_id);
}

bool Thread::finished() const
{
	return bool(WaitForSingleObject(m_handle, 0) == WAIT_OBJECT_0);
}

}
