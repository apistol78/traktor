#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedRandomGeometry.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedRandomGeometry, 4 > s_allocBoxedRandomGeometry;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.RandomGeometry", BoxedRandomGeometry, BoxedRandom)

BoxedRandomGeometry::BoxedRandomGeometry(const RandomGeometry& value)
:	m_value(value)
{
}

BoxedRandomGeometry::BoxedRandomGeometry(uint32_t seed)
:	m_value(seed)
{
}

std::wstring BoxedRandomGeometry::toString() const
{
	return L"(random geometry)";
}

void* BoxedRandomGeometry::operator new (size_t size)
{
	return s_allocBoxedRandomGeometry.alloc();
}

void BoxedRandomGeometry::operator delete (void* ptr)
{
	s_allocBoxedRandomGeometry.free(ptr);
}

}
