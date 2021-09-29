#include <time.h>
#include "Core/Timer/Timer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Timer", Timer, Object)

Timer::Timer()
{
	reset();
}

void Timer::reset()
{
	m_first = 0.0;
	m_last = 0.0;
	m_first = getElapsedTime();
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
	double current = getElapsedTime();
	double delta = current - m_last;
	m_last = current;
	return delta;
}

}
