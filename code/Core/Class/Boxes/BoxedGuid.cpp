#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedGuid.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedGuid, 512 > s_allocBoxedGuid;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Guid", BoxedGuid, Boxed)

BoxedGuid::BoxedGuid()
{
}

BoxedGuid::BoxedGuid(const Guid& value)
:	m_value(value)
{
}

BoxedGuid::BoxedGuid(const std::wstring& value)
:	m_value(value)
{
}

std::wstring BoxedGuid::toString() const
{
	return m_value.format();
}

void* BoxedGuid::operator new (size_t size)
{
	return s_allocBoxedGuid.alloc();
}

void BoxedGuid::operator delete (void* ptr)
{
	s_allocBoxedGuid.free(ptr);
}

}
