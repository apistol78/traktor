/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Platform.h"
#include "Core/Thread/Event.h"

namespace traktor
{
	namespace
	{

const uint32_t c_broadcast = ~0UL;

struct Internal
{
	CRITICAL_SECTION lock;
	HANDLE handle;
	uint32_t listeners;
	uint32_t signals;
};

	}

Event::Event()
{
	Internal* in = new Internal();
	InitializeCriticalSection(&in->lock);
	in->handle = CreateEvent(NULL, TRUE, FALSE, NULL);
	in->listeners = 0;
	in->signals = 0;
	m_handle = in;
}

Event::~Event()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	CloseHandle(in->handle);
	delete in;
}

void Event::pulse(int count)
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	EnterCriticalSection(&in->lock);

	in->signals += count;
	SetEvent(in->handle);

	LeaveCriticalSection(&in->lock);
}

void Event::broadcast()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	EnterCriticalSection(&in->lock);

	in->signals = c_broadcast;
	SetEvent(in->handle);

	LeaveCriticalSection(&in->lock);
}

void Event::reset()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	EnterCriticalSection(&in->lock);

	in->signals = 0;
	ResetEvent(in->handle);

	LeaveCriticalSection(&in->lock);
}

bool Event::wait(int timeout)
{
	bool result = false;

	Internal* in = reinterpret_cast< Internal* >(m_handle);
	InterlockedIncrement((LPLONG)&in->listeners);

#if !defined(_XBOX)
	MMRESULT mmresult = timeBeginPeriod(1);
#endif

	for (;;)
	{
		result = bool(WaitForSingleObject(in->handle, timeout >= 0 ? timeout : INFINITE) == WAIT_OBJECT_0);
		if (!result)
			break;

		EnterCriticalSection(&in->lock);

		if (in->signals != 0)
		{
			if (in->signals != c_broadcast)
				in->signals--;
			else
			{
				if (in->listeners <= 1)
					in->signals = 0;
			}

			if (in->signals == 0)
				ResetEvent(in->handle);

			LeaveCriticalSection(&in->lock);
			break;
		}

		LeaveCriticalSection(&in->lock);
	}

#if !defined(_XBOX)
	if (mmresult == TIMERR_NOERROR)
		timeEndPeriod(1);
#endif

	InterlockedDecrement((LPLONG)&in->listeners);
	return result;
}

}