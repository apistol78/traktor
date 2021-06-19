#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedIntervalTransform.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedIntervalTransform, 16 > s_allocBoxedIntervalTransform;
	
	}
T_IMPLEMENT_RTTI_CLASS(L"traktor.Transform", BoxedIntervalTransform, Boxed)

BoxedIntervalTransform::BoxedIntervalTransform(const IntervalTransform& value)
:	m_value(value)
{
}

Transform BoxedIntervalTransform::get(float interval) const
{
	return m_value.get(interval);
}

std::wstring BoxedIntervalTransform::toString() const
{
	return L"(interval transform)";
}

void* BoxedIntervalTransform::operator new (size_t size)
{
	return s_allocBoxedIntervalTransform.alloc();
}

void BoxedIntervalTransform::operator delete (void* ptr)
{
	s_allocBoxedIntervalTransform.free(ptr);
}

}
