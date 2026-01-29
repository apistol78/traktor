/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Object.h"

#if defined(__clang__) || defined (__GNUC__)
#	define ATTRIBUTE_NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
#elif defined(_MSC_VER)
#	define ATTRIBUTE_NO_SANITIZE_ADDRESS __declspec(no_sanitize_address)
#else
#	define ATTRIBUTE_NO_SANITIZE_ADDRESS
#endif

namespace traktor
{
	namespace
	{

#pragma pack(1)

const uint32_t c_magic = 'TRKT';

/*! Prepended on all heap allocated objects.
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

static std::atomic< int32_t > s_heapObjectCount(0);

ATTRIBUTE_NO_SANITIZE_ADDRESS
inline bool isObjectHeapAllocated(const void* ptr)
{
	const ObjectHeader* header = reinterpret_cast< const ObjectHeader* >(ptr) - 1;
	return bool(header->magic == c_magic);
}

	}

T_IMPLEMENT_RTTI_CLASS_ROOT(L"traktor.Object", Object)

#if defined(_DEBUG)
void Object::addRef(void* owner) const
{
	++m_refCount;
}

void Object::release(void* owner) const
{
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

	ObjectHeader* header = static_cast< ObjectHeader* >(getAllocator()->alloc(size + objectHeaderSize, 16, "Object"));
	T_FATAL_ASSERT_M (header, L"Out of memory (object)");
	header->magic = c_magic;

	Object* object = reinterpret_cast< Object* >(header + 1);

	s_heapObjectCount++;
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
		T_ASSERT(header->magic == c_magic);

		IAllocator* allocator = getAllocator();
		allocator->free(header);

		s_heapObjectCount--;
	}
}

void Object::operator delete (void* ptr, void* memory)
{
}

int32_t Object::getHeapObjectCount()
{
	return s_heapObjectCount;
}

void Object::finalRelease() const noexcept
{
	if (isObjectHeapAllocated(this))
		delete this;
}

}
