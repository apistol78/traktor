#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedRange.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedRange, 256 > s_allocBoxedRange;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Range", BoxedRange, Boxed)

BoxedRange::BoxedRange()
{
}

std::wstring BoxedRange::toString() const
{
	StringOutputStream ss;
	ss << m_min.getWideString() << L" - " << m_max.getWideString();
	return ss.str();
}

void* BoxedRange::operator new (size_t size)
{
	return s_allocBoxedRange.alloc();
}

void BoxedRange::operator delete (void* ptr)
{
	s_allocBoxedRange.free(ptr);
}

}
