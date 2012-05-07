#include "Core/Thread/Atomic.h"

namespace traktor
{

int32_t Atomic::increment(int32_t& value)
{
	return __sync_fetch_and_add(&value, 1) + 1;
}

int32_t Atomic::decrement(int32_t& value)
{
	return __sync_fetch_and_sub(&value, 1) - 1;
}

int32_t Atomic::add(int32_t& value, int32_t delta)
{
    return __sync_fetch_and_add(&value, delta) + delta;
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
	uint32_t cv = s; s = v;
	return cv;
}

uint64_t Atomic::exchange(uint64_t& s, uint64_t v)
{
	uint64_t cv = s; s = v;
	return cv;
}

}
