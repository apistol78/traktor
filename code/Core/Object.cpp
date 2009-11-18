#include "Core/Object.h"
#include "Core/Memory/Alloc.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS_ROOT(L"traktor.Object", Object)

Object::Object()
:	m_heap(false)
{
	const uint32_t tag = *(reinterpret_cast< const uint32_t* >(this) - 1);
	m_heap = bool(tag == 'TRKT');
}

Object::~Object()
{
	T_ASSERT_M (
		m_refCount == 0,
		L"Object destroyed prematurely;\n"
		L"Make sure no reference exist to stack objects"
	);
}

void Object::addRef() const
{
	++m_refCount;
}

void Object::release() const
{
	if (--m_refCount == 0)
	{
		if (m_heap)
			delete this;
	}
}

void* Object::operator new (size_t size)
{
	uint32_t* ptr = static_cast< uint32_t* >(Alloc::acquire(size + sizeof(uint32_t)));
	if (ptr)
	{
		*ptr++ = 'TOBJ';
		return ptr;
	}
	else
		return 0;
}

void Object::operator delete (void* ptr)
{
	if (ptr)
	{
		uint32_t* real = static_cast< uint32_t* >(ptr) - 1;
		Alloc::free(ptr);
	}
}

void* Object::operator new[] (size_t)
{
	T_FATAL_ERROR;
	return 0;
}

void Object::operator delete[] (void*)
{
	T_FATAL_ERROR;
}

}
