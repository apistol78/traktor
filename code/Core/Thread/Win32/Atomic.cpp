#include "Core/Platform.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

int32_t Atomic::increment(int32_t& value)
{
	return InterlockedIncrement((LPLONG)&value);
}

int32_t Atomic::decrement(int32_t& value)
{
	return InterlockedDecrement((LPLONG)&value);
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
	LONG p = InterlockedExchange((LPLONG)&s, *(LPLONG)&v);
	return *(uint32_t*)&p;
}

uint64_t Atomic::exchange(uint64_t& s, uint64_t v)
{
#if defined(_WIN64) && defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600
	LONGLONG p = InterlockedExchange64((LONGLONG*)&s, *(LONGLONG*)&v);
	return *(uint64_t*)&p;
#else
	static Semaphore lock;
	lock.acquire();
	uint64_t o = s; s = v;
	lock.release();
	return o;
#endif
}

}
