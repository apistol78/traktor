#include "Core/IObjectRefDebugger.h"
#include "Core/Object.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/FastAllocator.h"
#include "Core/Memory/StdAllocator.h"
#include "Core/Memory/TrackAllocator.h"
#include "Core/Misc/AutoPtr.h"

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

IAllocator* getAllocator()
{
	static AutoPtr< IAllocator > s_allocator;
	if (!s_allocator.ptr())
	{
#if !defined(_DEBUG)
		s_allocator.reset(new FastAllocator(
			new StdAllocator()
		));
#else
		s_allocator.reset(new TrackAllocator(
			new StdAllocator()
		));
#endif
	}
	return s_allocator.ptr();
}

	}

T_IMPLEMENT_RTTI_CLASS_ROOT(L"traktor.Object", Object)

IObjectRefDebugger* Object::ms_refDebugger = 0;

void Object::addRef(void* owner) const
{
	++m_refCount;

#if defined(_DEBUG)
	if (ms_refDebugger)
		ms_refDebugger->addObjectRef(owner, (void*)this);
#endif
}

void Object::release(void* owner) const
{
#if defined(_DEBUG)
	if (ms_refDebugger)
		ms_refDebugger->removeObjectRef(owner, (void*)this);
#endif

	if (--m_refCount == 0)
	{
		if (isObjectHeapAllocated(this))
			delete this;
	}
}

void* Object::operator new (size_t size)
{
	const size_t objectHeaderSize = sizeof(ObjectHeader);
	
	IAllocator* allocator = getAllocator();

	ObjectHeader* header = static_cast< ObjectHeader* >(allocator->alloc(size + objectHeaderSize, 16, "Object"));
	T_FATAL_ASSERT_M (header, L"Out of memory");

	header->magic = c_magic;

	Object* object = reinterpret_cast< Object* >(header + 1);

#if defined(_DEBUG)
	if (ms_refDebugger)
		ms_refDebugger->addObject(object, size);
#endif

	return object;
}

void Object::operator delete (void* ptr)
{
	if (ptr)
	{
		ObjectHeader* header = static_cast< ObjectHeader* >(ptr) - 1;
		T_ASSERT (header->magic == c_magic);

#if defined(_DEBUG)
		if (ms_refDebugger)
			ms_refDebugger->removeObject(ptr);
#endif

		IAllocator* allocator = getAllocator();
		allocator->free(header);
	}
}

int32_t Object::getReferenceCount() const
{
	return m_refCount;
}

void Object::setReferenceDebugger(IObjectRefDebugger* refDebugger)
{
	ms_refDebugger = refDebugger;
}

}
