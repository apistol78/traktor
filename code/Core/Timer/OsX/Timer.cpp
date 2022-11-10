/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <mach/mach.h>
#include <mach/mach_time.h>
#include "Core/Timer/Timer.h"

namespace traktor
{
	namespace
	{

double absoluteToSeconds(uint64_t abs)
{
	static mach_timebase_info_data_t tb = { 0, 0 };
	if (tb.denom == 0)
		mach_timebase_info(&tb);
	uint64_t nano = abs * tb.numer / tb.denom;
	return double(nano) / 1e9;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Timer", Timer, Object)

Timer::Timer()
{
	reset();
}

void Timer::reset()
{
	m_first = mach_absolute_time();
	m_last = m_first;
}

double Timer::getElapsedTime() const
{
	uint64_t current = mach_absolute_time();
	uint64_t elapsed = current - m_first;
	return absoluteToSeconds(elapsed);
}

double Timer::getDeltaTime()
{
	uint64_t current = mach_absolute_time();
	uint64_t delta = current - m_last; m_last = current;
	return absoluteToSeconds(delta);
}

}
