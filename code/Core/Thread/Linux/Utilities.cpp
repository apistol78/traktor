/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <sys/time.h>
#include "Core/Thread/Linux/Utilities.h"

namespace traktor
{
    namespace
    {

const int64_t c_nanosPerSecond = 1000000000;

    }

void addNanoSecToTimeSpec(struct timespec* r, int64_t b)
{
	r->tv_sec = r->tv_sec + (b / c_nanosPerSecond);
	r->tv_nsec = r->tv_nsec + (b % c_nanosPerSecond);

	if (r->tv_nsec >= c_nanosPerSecond)
    {
		r->tv_sec++;
		r->tv_nsec -= c_nanosPerSecond;
	}
    else if (r->tv_nsec < 0)
    {
		r->tv_sec--;
		r->tv_nsec += c_nanosPerSecond;
	}
}

void addMilliSecToTimeSpec(struct timespec* r, int64_t b)
{
	addNanoSecToTimeSpec(r, b * 1000000);
}

}
