#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedVector2.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/Format.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedVector2, 4096 > s_allocBoxedVector2;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Vector2", BoxedVector2, Boxed)

std::wstring BoxedVector2::toString() const
{
	StringOutputStream ss;
	ss << m_value;
	return ss.str();
}

void* BoxedVector2::operator new (size_t size)
{
	return s_allocBoxedVector2.alloc();
}

void BoxedVector2::operator delete (void* ptr)
{
	s_allocBoxedVector2.free(ptr);
}

}
