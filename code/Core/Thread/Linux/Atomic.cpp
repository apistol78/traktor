#include "Core/Thread/Atomic.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace
	{

Semaphore g_lock;

	}

int32_t Atomic::increment(int32_t& value)
{
	Acquire< Semaphore > lock(g_lock);
	return value++;
}

int32_t Atomic::decrement(int32_t& value)
{
	Acquire< Semaphore > lock(g_lock);
	return value--;
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
	Acquire< Semaphore > lock(g_lock);
	uint32_t cv = s; s = v;
	return cv;
}

}
