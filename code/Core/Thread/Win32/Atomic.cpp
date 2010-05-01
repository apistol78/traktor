#if !defined(WINCE)
#	include <intrin.h>
#endif
#include "Core/Platform.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

int32_t Atomic::increment(int32_t& value)
{
#if !defined(WINCE)
	return _InterlockedIncrement((LPLONG)&value);
#else
	return InterlockedIncrement((LPLONG)&value);
#endif
}

int32_t Atomic::decrement(int32_t& value)
{
#if !defined(WINCE)
	return _InterlockedDecrement((LPLONG)&value);
#else
	return InterlockedDecrement((LPLONG)&value);
#endif
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
#if !defined(WINCE)
	LONG p = _InterlockedExchange((LPLONG)&s, *(LPLONG)&v);
#else
	LONG p = InterlockedExchange((LPLONG)&s, *(LPLONG)&v);
#endif
	return *(uint32_t*)&p;
}

uint64_t Atomic::exchange(uint64_t& s, uint64_t v)
{
#if defined(_WIN64) && defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600
	LONGLONG p = _InterlockedExchange64((LONGLONG*)&s, *(LONGLONG*)&v);
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
