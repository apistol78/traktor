/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/Align.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IAllocator;

/*! Pool allocator.
 * \ingroup Core
 *
 * Pool allocator is designed to allocate continuous chunks of
 * memory quickly. All memory are released as soon as the pool
 * is released thus there are no way of freeing individual
 * allocations.
 *
 * In order to make this pool a bit more efficient it
 * supports allocations scopes; it can nest several allocations
 * in scopes and as soon as a scope is leaved objects allocated
 * in that scope are destroyed.
 */
class T_DLLCLASS PoolAllocator : public Object
{
	T_RTTI_CLASS;

public:
	PoolAllocator();

	explicit PoolAllocator(IAllocator* allocator, uint32_t totalSize);

	explicit PoolAllocator(uint32_t totalSize);

	explicit PoolAllocator(void* heap, uint32_t totalSize);

	virtual ~PoolAllocator();

	/*! Enter allocation scope. */
	void enter();

	/*! Leave allocation scope. */
	void leave();

	/*! Allocate chunk.
	 *
	 * \param size Size of chunk.
	 * \param align Alignment of chunk.
	 * \return Pointer to chunk.
	 */
	[[nodiscard]] void* alloc(uint32_t size, uint32_t align);

	/*! Allocate object.
	 *
	 * \return Pointer to object.
	 */
	template < typename Type >
	[[nodiscard]] Type* alloc()
	{
		void* ptr = alloc((uint32_t)sizeof(Type), (uint32_t)alignOf< Type >());
		return new (ptr) Type();
	}

	/*! Allocate array of objects.
	 *
	 * \param count Number of objects.
	 * \return Pointer to first object.
	 */
	template < typename Type >
	[[nodiscard]] Type* alloc(uint32_t count)
	{
		if (!count)
			return nullptr;

		void* ptr = alloc((uint32_t)sizeof(Type) * count, (uint32_t)alignOf< Type >());
		return new (ptr) Type [count];
	}

	/*! Allocate array of object pointers.
	 *
	 * \param count Number of object pointers.
	 * \return Pointer to first object pointer.
	 */
	template < typename Type >
	[[nodiscard]] Type** allocArray(uint32_t count)
	{
		if (!count)
			return nullptr;

		void* ptr = alloc((uint32_t)sizeof(Type*) * count, (uint32_t)alignOf< Type* >());
		return static_cast< Type** >(ptr);
	}

private:
	IAllocator* m_allocator;
	bool m_ownAllocator;
	uint32_t m_totalSize;
	uint8_t* m_head;
	uint8_t* m_tail;
	AlignedVector< uint8_t* > m_scope;
	AlignedVector< uint8_t* > m_heaps;
};

}

