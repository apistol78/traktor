/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sys/sys_time.h>
#include <sys/time_util.h>
#include "Core/Timer/Timer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Timer", Timer, Object)

Timer::Timer()
:	m_frequency(0)
,	m_first(0)
,	m_last(0)
,	m_paused(true)
{
	m_frequency = sys_time_get_timebase_frequency();
}

void Timer::start()
{
	SYS_TIMEBASE_GET(m_first);
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
	uint64_t curr;
	SYS_TIMEBASE_GET(curr);
	return double(curr - m_first) / m_frequency;
}

double Timer::getDeltaTime()
{
	uint64_t curr;
	SYS_TIMEBASE_GET(curr);
	
	double delta = double(curr - m_last) / m_frequency;
	m_last = curr;

	return delta;
}

}
