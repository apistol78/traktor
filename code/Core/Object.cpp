/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/IObjectRefDebugger.h"
#include "Core/Object.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"

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

static int32_t s_heapObjectCount = 0;

inline bool isObjectHeapAllocated(const void* ptr)
{
	const ObjectHeader* header = reinterpret_cast< const ObjectHeader* >(ptr) - 1;
	return bool(header->magic == c_magic);
}

	}

T_IMPLEMENT_RTTI_CLASS_ROOT(L"traktor.Object", Object)

IObjectRefDebugger* Object::ms_refDebugger = 0;

#if defined(_DEBUG)
void Object::addRef(void* owner) const
{
	++m_refCount;
	if (ms_refDebugger)
		ms_refDebugger->addObjectRef(owner, (void*)this);
}

void Object::release(void* owner) const
{
	if (ms_refDebugger)
		ms_refDebugger->removeObjectRef(owner, (void*)this);
	if (--m_refCount == 0)
		finalRelease();
}

int32_t Object::getReferenceCount() const
{
	return m_refCount;
}
#endif

void* Object::operator new (size_t size)
{
	const size_t objectHeaderSize = sizeof(ObjectHeader);
	
	IAllocator* allocator = getAllocator();

	ObjectHeader* header = static_cast< ObjectHeader* >(allocator->alloc(size + objectHeaderSize, 16, "Object"));
	T_FATAL_ASSERT_M (header, L"Out of memory (object)");

	header->magic = c_magic;

	Object* object = reinterpret_cast< Object* >(header + 1);

#if defined(_DEBUG)
	if (ms_refDebugger)
		ms_refDebugger->addObject(object, size);
#endif

	Atomic::increment(s_heapObjectCount);

	return object;
}

void* Object::operator new (size_t size, void* memory)
{
	return memory;
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

		Atomic::decrement(s_heapObjectCount);
	}
}

void Object::operator delete (void* ptr, void* memory)
{
}

void Object::setReferenceDebugger(IObjectRefDebugger* refDebugger)
{
	ms_refDebugger = refDebugger;
}

int32_t Object::getHeapObjectCount()
{
	return s_heapObjectCount;
}

void Object::finalRelease() const
{
	if (isObjectHeapAllocated(this))
		delete this;
}

}
