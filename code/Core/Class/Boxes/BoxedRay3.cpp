#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedRay3.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/Format.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedRay3, 8 > s_allocBoxedRay3;
	
	}
T_IMPLEMENT_RTTI_CLASS(L"traktor.Ray3", BoxedRay3, Boxed)

BoxedRay3::BoxedRay3(const Ray3& value)
:	m_value(value)
{
}

std::wstring BoxedRay3::toString() const
{
	StringOutputStream ss;
	ss << L"(" << m_value.origin << L") - (" << m_value.direction << L")";
	return ss.str();
}

void* BoxedRay3::operator new (size_t size)
{
	return s_allocBoxedRay3.alloc();
}

void BoxedRay3::operator delete (void* ptr)
{
	s_allocBoxedRay3.free(ptr);
}

}
