#include <cell/atomic.h>
#include "Core/Thread/Atomic.h"

namespace traktor
{

int32_t Atomic::increment(int32_t& value)
{
#if !defined(SPU)
	cellAtomicIncr32((uint32_t*)&value);
#endif
	return value;
}

int32_t Atomic::decrement(int32_t& value)
{
#if !defined(SPU)
	cellAtomicDecr32((uint32_t*)&value);
#endif
	return value;
}

int32_t Atomic::add(int32_t& value, int32_t delta)
{
#if !defined(SPU)
	cellAtomicAdd32((uint32_t*)&value, delta);
#endif
	return value;
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
	uint32_t old = s;
#if !defined(SPU)
	cellAtomicStore32(&s, v);
#endif
	return old;
}

uint64_t Atomic::exchange(uint64_t& s, uint64_t v)
{
	uint64_t old = s;
#if !defined(SPU)
	cellAtomicStore64(&s, v);
#endif
	return old;
}

}
