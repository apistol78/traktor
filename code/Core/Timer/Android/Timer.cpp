/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sys/time.h>
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
}

void Timer::start()
{
	timeval tv;
	gettimeofday(&tv, 0);

	m_first = int64_t(tv.tv_sec) * 1e6 + tv.tv_usec;
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
	timeval tv;
	gettimeofday(&tv, 0);

	int64_t current = int64_t(tv.tv_sec) * 1e6 + tv.tv_usec;
	int64_t elapsed = current - m_first;

	return double(elapsed) / 1e6;
}

double Timer::getDeltaTime()
{
	timeval tv;
	gettimeofday(&tv, 0);

	int64_t current = int64_t(tv.tv_sec) * 1e6 + tv.tv_usec;

	double delta = double(current - m_last) / 1e6;
	m_last = current;

	return delta;
}

}
