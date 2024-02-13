/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
	timespec ts = { 0 };
	clock_gettime(CLOCK_MONOTONIC, &ts);
	const double current = ts.tv_sec + ts.tv_nsec / 1e9;
	return current - m_first;
}

double Timer::getDeltaTime()
{
	const double current = getElapsedTime();
	const double delta = current - m_last;
	m_last = current;
	return delta;
}

}
