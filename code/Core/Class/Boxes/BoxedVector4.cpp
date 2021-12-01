#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/Format.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedVector4, 32768 > s_allocBoxedVector4;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Vector4", BoxedVector4, Boxed)

std::wstring BoxedVector4::toString() const
{
	StringOutputStream ss;
	ss << m_value;
	return ss.str();
}

void* BoxedVector4::operator new (size_t size)
{
	return s_allocBoxedVector4.alloc();
}

void BoxedVector4::operator delete (void* ptr)
{
	s_allocBoxedVector4.free(ptr);
}

}
