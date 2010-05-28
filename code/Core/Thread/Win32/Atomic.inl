#if !defined(WINCE)
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
#if !defined(WINCE)
	return _InterlockedIncrement((long volatile*)&value);
#else
	return InterlockedIncrement((LPLONG)&value);
#endif
}

int32_t Atomic::decrement(int32_t& value)
{
#if !defined(WINCE)
	return _InterlockedDecrement((long volatile*)&value);
#else
	return InterlockedDecrement((LPLONG)&value);
#endif
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
#if !defined(WINCE)
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

}
