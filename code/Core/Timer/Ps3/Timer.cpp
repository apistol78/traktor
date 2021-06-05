#include <sys/sys_time.h>
#include <sys/time_util.h>
#include "Core/Timer/Timer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Timer", Timer, Object)

Timer::Timer()
{
	m_frequency = sys_time_get_timebase_frequency();
	reset();
}

void Timer::reset()
{
	SYS_TIMEBASE_GET(m_first);
	m_last = m_first;
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
