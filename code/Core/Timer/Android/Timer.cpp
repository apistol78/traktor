#include <time.h>
#include "Core/Timer/Timer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Timer", Timer, Object)

Timer::Timer()
{
}

void Timer::start()
{
	m_first = 0.0;
	m_last = 0.0;
	m_first = getElapsedTime();
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
	timespec ts = {};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	double current = ts.tv_sec + ts.tv_nsec / 1e9;
	return current - m_first;
}

double Timer::getDeltaTime()
{
	timespec ts = {};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	double current = ts.tv_sec + ts.tv_nsec / 1e9;
	double delta = current - m_last;
	m_last = current;
	return delta;
}

}
