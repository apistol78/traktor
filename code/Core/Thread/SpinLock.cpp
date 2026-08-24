/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/SpinLock.h"

#include "Core/Thread/Atomic.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"

#if defined(_MSC_VER)
#	include <intrin.h>
#	if defined(_M_IX86) || defined(_M_X64)
#		define T_CPU_RELAX() _mm_pause()
#	elif defined(_M_ARM) || defined(_M_ARM64)
#		define T_CPU_RELAX() __yield()
#	else
#		define T_CPU_RELAX() ((void)0)
#	endif
#elif defined(__i386__) || defined(__x86_64__)
#	include <immintrin.h>
#	define T_CPU_RELAX() _mm_pause()
#elif defined(__aarch64__) || defined(__arm__)
#	define T_CPU_RELAX() __asm__ __volatile__("yield")
#else
#	define T_CPU_RELAX() ((void)0)
#endif

namespace traktor
{
namespace
{

Timer s_timer;
const uint32_t c_minRelaxCount = 4;
const uint32_t c_maxRelaxCount = 256;

}

SpinLock::SpinLock()
	: m_lock(0)
{
}

SpinLock::~SpinLock()
{
	release();
}

bool SpinLock::wait(int32_t timeout)
{
	if (Atomic::compareAndSwap(m_lock, 0, 1) == 0)
		return true;

	// A timeout of zero or less means wait indefinitely.
	const bool expires = (timeout > 0);
	const double expireAt = expires ? s_timer.getElapsedTime() + timeout / 1000.0 : 0.0;

	Thread* thread = nullptr;
	uint32_t relaxCount = c_minRelaxCount;

	for (;;)
	{
		for (uint32_t i = 0; i < relaxCount; ++i)
			T_CPU_RELAX();

		if (Atomic::compareAndSwap(m_lock, 0, 1) == 0)
			return true;

		if (relaxCount < c_maxRelaxCount)
			relaxCount *= 2;
		else
		{
			// Hand the core over to a thread which can make progress.
			if (!thread)
				thread = ThreadManager::getInstance().getCurrentThread();
			thread->yield();
		}

		if (expires && s_timer.getElapsedTime() >= expireAt)
			return false;
	}
}

void SpinLock::release()
{
	Atomic::exchange(m_lock, 0);
}

}
