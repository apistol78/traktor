#include <sce_atomic.h>
#include "Core/Thread/Atomic.h"

namespace traktor
{

int32_t Atomic::increment(int32_t& value)
{
	return sceAtomicIncrement32(&value);
}

int32_t Atomic::decrement(int32_t& value)
{
	return sceAtomicDecrement32(&value);
}

int32_t Atomic::add(int32_t& value, int32_t delta)
{
	return sceAtomicAdd32(&value, delta);
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
	return sceAtomicExchange32((volatile int32_t*)&s, v);
}

uint64_t Atomic::exchange(uint64_t& s, uint64_t v)
{
	return sceAtomicExchange64((volatile int64_t*)&s, v);
}

int32_t Atomic::compareAndSwap(int32_t& value, int32_t compareTo, int32_t replaceWithIfEqual)
{
	return sceAtomicCompareAndSwap32(&value, compareTo, replaceWithIfEqual);
}

}
