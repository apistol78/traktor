#include <libkern/OSAtomic.h>
#include "Core/Thread/Atomic.h"

namespace traktor
{

int32_t Atomic::increment(volatile int32_t& value)
{
	return OSAtomicIncrement32((int32_t*)(&value));
}

int32_t Atomic::decrement(volatile int32_t& value)
{
	return OSAtomicDecrement32((int32_t*)(&value));
}

}
