#include <kernel.h>
#include "Core/Timer/Timer.h"

namespace traktor
{
	namespace
	{

uint64_t s_frequency = 0;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Timer", Timer, Object)

Timer::Timer()
:	m_frequency(0)
,	m_first(0)
,	m_last(0)
,	m_paused(true)
{
	if (!s_frequency)
		s_frequency = sceKernelGetProcessTimeCounterFrequency();

	m_frequency = s_frequency;
}

void Timer::start()
{
	m_first = sceKernelGetProcessTimeCounter();
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
	uint64_t curr = sceKernelGetProcessTimeCounter();
	return double(curr - m_first) / m_frequency;
}

double Timer::getDeltaTime()
{
	uint64_t curr = sceKernelGetProcessTimeCounter();
	double delta = double(curr - m_last) / m_frequency;
	m_last = curr;
	return delta >= 0.0 ? delta : 0.0;
}
	
}

