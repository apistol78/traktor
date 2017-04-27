/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
:	m_frequency(0)
,	m_first(0)
,	m_last(0)
,	m_paused(true)
{
}

void Timer::start()
{
	m_first = mach_absolute_time();
	m_last = m_first;
	m_paused = false;
}

void Timer::pause()
{
	m_paused = true;
}

void Timer::stop()
{
	m_paused = true;
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
