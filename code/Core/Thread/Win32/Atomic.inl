/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <intrin.h>
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

int32_t Atomic::increment(int32_t& value)
{
	return _InterlockedIncrement((long volatile*)&value);
}

int32_t Atomic::decrement(int32_t& value)
{
	return _InterlockedDecrement((long volatile*)&value);
}

int32_t Atomic::add(int32_t& value, int32_t delta)
{
	return _InterlockedExchangeAdd((long volatile*)&value, delta);
}

int64_t Atomic::add(int64_t& value, int64_t delta)
{
	return _InterlockedExchangeAdd64((__int64 volatile*)&value, delta);
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
	long p = _InterlockedExchange((long volatile*)&s, *(long*)&v);
	return *(uint32_t*)&p;
}

uint64_t Atomic::exchange(uint64_t& s, uint64_t v)
{
	__int64 p = _InterlockedExchange64((__int64 volatile*)&s, *(__int64*)&v);
	return *(uint64_t*)&p;
}

int32_t Atomic::compareAndSwap(int32_t& value, int32_t compareTo, int32_t replaceWithIfEqual)
{
	return _InterlockedCompareExchange((long volatile*)&value, replaceWithIfEqual, compareTo);
}

}
