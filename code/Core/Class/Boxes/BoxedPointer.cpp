#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedPointer.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedPointer, 32 > s_allocBoxedPointer;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Pointer", BoxedPointer, Boxed)

BoxedPointer::BoxedPointer()
:	m_ptr(nullptr)
{
}

std::wstring BoxedPointer::toString() const
{
	return L"(pointer)";
}

void* BoxedPointer::operator new (size_t size)
{
	return s_allocBoxedPointer.alloc();
}

void BoxedPointer::operator delete (void* ptr)
{
	s_allocBoxedPointer.free(ptr);
}

}
