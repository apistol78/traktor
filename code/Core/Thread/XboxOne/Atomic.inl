/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if !defined(_XBOX)
#	include <intrin.h>
#else
#	include "Core/Platform.h"
#endif
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

int32_t Atomic::increment(int32_t& value)
{
#if !defined(_XBOX)
	return _InterlockedIncrement((long volatile*)&value);
#else
	return InterlockedIncrement((LPLONG)&value);
#endif
}

int32_t Atomic::decrement(int32_t& value)
{
#if !defined(_XBOX)
	return _InterlockedDecrement((long volatile*)&value);
#else
	return InterlockedDecrement((LPLONG)&value);
#endif
}

int32_t Atomic::add(int32_t& value, int32_t delta)
{
#if !defined(_XBOX)
	return _InterlockedExchangeAdd((long volatile*)&value, delta);
#else
	return InterlockedExchangeAdd((LPLONG)&value, delta);
#endif
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
#if !defined(_XBOX)
	long p = _InterlockedExchange((long volatile*)&s, *(long*)&v);
#else
	LONG p = InterlockedExchange((LPLONG)&s, *(LPLONG)&v);
#endif
	return *(uint32_t*)&p;
}

uint64_t Atomic::exchange(uint64_t& s, uint64_t v)
{
#if defined(_WIN64) && defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600
	__int64 p = _InterlockedExchange64((__int64 volatile*)&s, *(__int64*)&v);
	return *(uint64_t*)&p;
#else
	static Semaphore lock;
	lock.wait();
	uint64_t o = s; s = v;
	lock.release();
	return o;
#endif
}

int32_t Atomic::compareAndSwap(int32_t& value, int32_t compareTo, int32_t replaceWithIfEqual)
{
#if !defined(_XBOX)
	return _InterlockedCompareExchange((long volatile*)&value, replaceWithIfEqual, compareTo);
#else
	return InterlockedCompareExchange((LPLONG)&value, replaceWithIfEqual, compareTo);
#endif
}

}
