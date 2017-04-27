/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Atomic.h"
#include "Core/Thread/SpinLock.h"
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
	if (timeout <= 0)
	{
		for (;;)
		{
			if (Atomic::compareAndSwap(m_lock, 0, 1) == 0)
				break;
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
		}
	}
	return true;
}

void SpinLock::release()
{
	Atomic::exchange(m_lock, 0);
}

}
