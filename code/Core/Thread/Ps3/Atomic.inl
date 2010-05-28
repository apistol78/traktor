#include <cell/atomic.h>
#include "Core/Thread/Atomic.h"

namespace traktor
{

int32_t Atomic::increment(int32_t& value)
{
	cellAtomicIncr32((uint32_t*)&value);
	return value;
}

int32_t Atomic::decrement(int32_t& value)
{
	cellAtomicDecr32((uint32_t*)&value);
	return value;
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
	uint32_t old = s;
	cellAtomicStore32(&s, v);
	return old;
}

uint64_t Atomic::exchange(uint64_t& s, uint64_t v)
{
	uint64_t old = s;
	cellAtomicStore64(&s, v);
	return old;
}

}
