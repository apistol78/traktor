/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <process.h>
#include "Core/Platform.h"
#include "Core/Thread/Thread.h"
#include "Core/Functor/Functor.h"
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
	Functor* functor = reinterpret_cast< Functor* >(lparam);
	(functor->operator())();
	return 0;
}

	}

Thread::Thread(Functor* functor, const std::wstring& name, int32_t hardwareCore)
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
}

bool Thread::start(Priority priority)
{
	if (m_handle)
		return false;

	m_handle = (void*)_beginthreadex(
		0,
		0,
		&threadProc,
		(void*)m_functor,
		CREATE_SUSPENDED,
		&m_id
	);
	if (!m_handle)
		return false;

#if defined(_XBOX)
	if (m_hardwareCore != -1)
	{
		DWORD result = XSetThreadProcessor(m_handle, m_hardwareCore);
		if (result == ~0U)
			return false;
	}
#endif

	THREADNAME_INFO threadInfo;
	threadInfo.dwType = 0x1000;
	threadInfo.szName = m_name.c_str();
	threadInfo.dwThreadID = m_id;
	threadInfo.dwFlags = 0;
	__try { RaiseException(0x406D1388, 0, sizeof(threadInfo) / sizeof(DWORD), (const ULONG_PTR*)&threadInfo); }
	__except (EXCEPTION_CONTINUE_EXECUTION) {}

#	if defined(_XBOX)
	PIXNameThread(m_name.c_str());
#	endif

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
#if !defined(_XBOX)
	MMRESULT result = TIMERR_NOCANDO;
	if (duration <= 10)
		result = timeBeginPeriod(1);
#endif

	Sleep(duration);

#if !defined(_XBOX)
	if (result == TIMERR_NOERROR)
		timeEndPeriod(1);
#endif
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
