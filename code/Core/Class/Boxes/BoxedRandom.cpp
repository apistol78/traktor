#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedRandom.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedRandom, 4 > s_allocBoxedRandom;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Random", BoxedRandom, Boxed)

BoxedRandom::BoxedRandom(const Random& value)
:	m_value(value)
{
}

BoxedRandom::BoxedRandom(uint32_t seed)
:	m_value(seed)
{
}

std::wstring BoxedRandom::toString() const
{
	return L"(random)";
}

void* BoxedRandom::operator new (size_t size)
{
	return s_allocBoxedRandom.alloc();
}

void BoxedRandom::operator delete (void* ptr)
{
	s_allocBoxedRandom.free(ptr);
}

}
