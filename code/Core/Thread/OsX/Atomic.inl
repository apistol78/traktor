#include <atomic>
#include "Core/Thread/Atomic.h"

namespace traktor
{

int32_t Atomic::increment(int32_t& value)
{
	std::atomic< int32_t > av(value);
	std::atomic_fetch_add_explicit(&av, 1, std::memory_order_relaxed);
	value = av.load();
	return av.load();
}

int32_t Atomic::decrement(int32_t& value)
{
	std::atomic< int32_t > av(value);
	std::atomic_fetch_sub_explicit(&av, 1, std::memory_order_relaxed);
	value = av.load();
	return av.load();
}

int32_t Atomic::add(int32_t& value, int32_t delta)
{
	std::atomic< int32_t > av(value);
	std::atomic_fetch_add_explicit(&av, delta, std::memory_order_relaxed);
	value = av.load();
	return av.load();
}

uint32_t Atomic::exchange(uint32_t& s, uint32_t v)
{
	uint32_t original = s;
	std::atomic< uint32_t > as(original);
	std::atomic_exchange_explicit(&as, v, std::memory_order_relaxed);
	s = as.load();
	return original;
}

uint64_t Atomic::exchange(uint64_t& s, uint64_t v)
{
	uint64_t original = s;
	std::atomic< uint64_t > as(original);
	std::atomic_exchange_explicit(&as, v, std::memory_order_relaxed);
	s = as.load();
	return original;
}

int32_t Atomic::compareAndSwap(int32_t& value, int32_t compareTo, int32_t replaceWithIfEqual)
{
	int32_t original = value;
	std::atomic< int32_t > av(original);
	std::atomic_compare_exchange_strong_explicit(
		&av,
		&compareTo,
		replaceWithIfEqual,
		std::memory_order_relaxed,
		std::memory_order_relaxed
	);
	return original;
}

}
