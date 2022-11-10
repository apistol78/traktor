/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Atomic.h"
#include "Core/Thread/SpinLock.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"

namespace traktor
{
	namespace
	{

Timer s_timer;

	}

SpinLock::SpinLock()
:	m_lock(0)
{
}

SpinLock::~SpinLock()
{
	release();
}

bool SpinLock::wait(int32_t timeout)
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();
	if (timeout <= 0)
	{
		for (;;)
		{
			if (Atomic::compareAndSwap(m_lock, 0, 1) == 0)
				break;
			thread->yield();
		}
	}
	else
	{
		double time = s_timer.getElapsedTime() + timeout / 1000.0;
		for (;;)
		{
			if (Atomic::compareAndSwap(m_lock, 0, 1) == 0)
				break;
			if (s_timer.getElapsedTime() >= time)
				return false;
			thread->yield();
		}
	}
	return true;
}

void SpinLock::release()
{
	Atomic::exchange(m_lock, 0);
}

}
