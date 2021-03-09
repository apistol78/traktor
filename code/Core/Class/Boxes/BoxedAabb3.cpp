#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedAabb3.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedAabb3, 64 > s_allocBoxedAabb3;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Aabb3", BoxedAabb3, Boxed)

BoxedAabb3::BoxedAabb3(const Aabb3& value)
:	m_value(value)
{
}

BoxedAabb3::BoxedAabb3(const BoxedVector4* min, const BoxedVector4* max)
:	m_value(min->unbox(), max->unbox())
{
}

Any BoxedAabb3::intersectRay(const BoxedVector4* origin, const BoxedVector4* direction) const
{
	Scalar distanceEnter, distanceExit;
	if (m_value.intersectRay(origin->unbox(), direction->unbox(), distanceEnter, distanceExit))
		return Any::fromFloat(distanceEnter);
	else
		return Any();
}

std::wstring BoxedAabb3::toString() const
{
	return L"(aabb3)";
}

void* BoxedAabb3::operator new (size_t size)
{
	return s_allocBoxedAabb3.alloc();
}

void BoxedAabb3::operator delete (void* ptr)
{
	s_allocBoxedAabb3.free(ptr);
}

}
