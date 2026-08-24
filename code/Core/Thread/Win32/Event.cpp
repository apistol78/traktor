/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Event.h"

#include "Core/System.h"

namespace traktor
{
namespace
{

const uint32_t c_broadcast = ~0UL;

struct Internal
{
	CRITICAL_SECTION lock;
	CONDITION_VARIABLE signaled;
	uint32_t listeners;
	uint32_t signals;
};

}

Event::Event()
{
	Internal* in = new Internal();
	InitializeCriticalSection(&in->lock);
	InitializeConditionVariable(&in->signaled);
	in->listeners = 0;
	in->signals = 0;
	m_handle = in;
}

Event::~Event()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	DeleteCriticalSection(&in->lock);
	delete in;
}

void Event::pulse(int32_t count)
{
	if (count <= 0)
		return;

	Internal* in = reinterpret_cast< Internal* >(m_handle);
	EnterCriticalSection(&in->lock);

	in->signals += count;
	const uint32_t waiting = in->listeners;
	const uint32_t wake = (uint32_t)count < waiting ? (uint32_t)count : waiting;

	LeaveCriticalSection(&in->lock);

	for (uint32_t i = 0; i < wake; ++i)
		WakeConditionVariable(&in->signaled);
}

void Event::broadcast()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	EnterCriticalSection(&in->lock);

	in->signals = c_broadcast;
	const bool wake = (in->listeners > 0);

	LeaveCriticalSection(&in->lock);

	if (wake)
		WakeAllConditionVariable(&in->signaled);
}

void Event::reset()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	EnterCriticalSection(&in->lock);

	in->signals = 0;

	LeaveCriticalSection(&in->lock);
}

bool Event::wait(int32_t timeout)
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	const DWORD duration = (timeout >= 0) ? (DWORD)timeout : INFINITE;

	EnterCriticalSection(&in->lock);

	in->listeners++;

	bool result = false;
	for (;;)
	{
		if (in->signals != 0)
		{
			if (in->signals != c_broadcast)
				in->signals--;
			else if (in->listeners <= 1)
				in->signals = 0;

			result = true;
			break;
		}

		if (!SleepConditionVariableCS(&in->signaled, &in->lock, duration))
			break;
	}

	in->listeners--;

	LeaveCriticalSection(&in->lock);
	return result;
}

}
