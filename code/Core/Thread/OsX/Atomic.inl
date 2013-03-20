#include <libkern/OSAtomic.h>
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

int32_t Atomic::increment(int32_t& value)
{
	return OSAtomicIncrement32((int32_t*)(&value));
}

int32_t Atomic::decrement(int32_t& value)
{
	return OSAtomicDecrement32((int32_t*)(&value));
}

int32_t Atomic::add(int32_t& value, int32_t delta)
{
	return OSAtomicAdd32(delta, (int32_t*)(&value));
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
	uint32_t old = s;
	OSAtomicCompareAndSwap32(*(int32_t*)&s, *(int32_t*)&v, (int32_t*)&s);
	return old;
}

uint64_t Atomic::exchange(uint64_t& s, uint64_t v)
{
	static Semaphore lock;
	lock.wait();
	uint64_t o = s; s = v;
	lock.release();
	return o;
}

int32_t Atomic::compareAndSwap(int32_t& value, int32_t compareTo, int32_t replaceWithIfEqual)
{
	OSAtomicCompareAndSwap32(*(int32_t*)&compareTo, *(int32_t*)&replaceWithIfEqual, (int32_t*)&value);
	return value;
}

}
