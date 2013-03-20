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
	return __sync_lock_test_and_set(&s, v);
}

uint64_t Atomic::exchange(uint64_t& s, uint64_t v)
{
	return __sync_lock_test_and_set(&s, v);
}

int32_t Atomic::compareAndSwap(int32_t& value, int32_t compareTo, int32_t replaceWithIfEqual)
{
	__sync_val_compare_and_swap(&value, compareTo, replaceWithIfEqual);
	return value;
}

}
