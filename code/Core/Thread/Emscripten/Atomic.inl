#include "Core/Thread/Atomic.h"

namespace traktor
{

int32_t Atomic::increment(int32_t& value)
{
	return ++value;
}

int32_t Atomic::decrement(int32_t& value)
{
	return --value;
}

int32_t Atomic::add(int32_t& value, int32_t delta)
{
	value += delta;
	return value;
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
	uint32_t old = s; s = v;
	return old;
}

uint64_t Atomic::exchange(uint64_t& s, uint64_t v)
{
	uint64_t old = s; s = v;
	return old;
}

int32_t Atomic::compareAndSwap(int32_t& value, int32_t compareTo, int32_t replaceWithIfEqual)
{
	if (value == compareTo)
		value = replaceWithIfEqual;
	return value;
}

}
