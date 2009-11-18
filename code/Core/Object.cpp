#include "Core/Object.h"
#include "Core/Memory/Alloc.h"

namespace traktor
{
	namespace
	{

#pragma pack(1)

const uint32_t c_magic = 'TRKT';

/*! \brief Prepended on all heap allocated objects.
 *
 * \note
 * Must be a multiple of maximum alignment requirement.
 */
struct ObjectHeader
{
	uint32_t magic;
	uint8_t reserved[12];
};

#pragma pack()

inline bool isObjectHeapAllocated(const void* ptr)
{
	const ObjectHeader* header = reinterpret_cast< const ObjectHeader* >(ptr) - 1;
	return bool(header->magic == c_magic);
}

	}

T_IMPLEMENT_RTTI_CLASS_ROOT(L"traktor.Object", Object)

Object::~Object()
{
	//T_ASSERT_M (
	//	m_refCount == 0,
	//	L"Object destroyed prematurely;\n"
	//	L"Make sure no reference exist to stack objects"
	//);
}

void Object::addRef() const
{
	++m_refCount;
}

void Object::release() const
{
	if (--m_refCount == 0)
	{
		if (isObjectHeapAllocated(this))
			delete this;
	}
}

void* Object::operator new (size_t size)
{
	const size_t objectHeaderSize = sizeof(ObjectHeader);
	ObjectHeader* header = static_cast< ObjectHeader* >(Alloc::acquireAlign(size + objectHeaderSize, 16));
	if (header)
	{
		header->magic = c_magic;

#if defined(_DEBUG)
		for (int i = 0; i < sizeof_array(header->reserved); ++i)
			header->reserved[i] = 0;
#endif

		return header + 1;
	}
	else
		return 0;
}

void Object::operator delete (void* ptr)
{
	if (ptr)
	{
		ObjectHeader* header = static_cast< ObjectHeader* >(ptr) - 1;
		T_ASSERT (header->magic == c_magic);
		Alloc::freeAlign(header);
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
