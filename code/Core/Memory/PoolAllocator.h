/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_PoolAllocator_H
#define traktor_PoolAllocator_H

#include <list>
#include <stack>
#include "Core/Object.h"
#include "Core/Ref.h"

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

/*! \brief Pool allocator.
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

	PoolAllocator(IAllocator* allocator, uint32_t totalSize);

	PoolAllocator(uint32_t totalSize);

	PoolAllocator(void* heap, uint32_t totalSize);

	virtual ~PoolAllocator();

	/*! \brief Enter allocation scope. */
	void enter();

	/*! \brief Leave allocation scope. */
	void leave();

	/*! \brief Allocate chunk.
	 *
	 * \param size Size of chunk.
	 * \param align Alignment of chunk.
	 * \return Pointer to chunk.
	 */
	void* alloc(uint32_t size, uint32_t align);

	/*! \brief Allocate object.
	 *
	 * \return Pointer to object.
	 */
	template < typename Type >
	Type* alloc()
	{
		void* ptr = alloc(sizeof(Type), alignOf< Type >());
		return new (ptr) Type();
	}

	/*! \brief Allocate array of objects.
	 *
	 * \param count Number of objects.
	 * \return Pointer to first object.
	 */
	template < typename Type >
	Type* alloc(uint32_t count)
	{
		if (!count)
			return 0;

		void* ptr = alloc(sizeof(Type) * count, alignOf< Type >());
		return new (ptr) Type [count];
	}

	/*! \brief Allocate array of object pointers.
	 *
	 * \param count Number of object pointers.
	 * \return Pointer to first object pointer.
	 */
	template < typename Type >
	Type** allocArray(uint32_t count)
	{
		if (!count)
			return 0;

		void* ptr = alloc(sizeof(Type*) * count, alignOf< Type* >());
		return static_cast< Type** >(ptr);
	}

private:
	Ref< IAllocator > m_allocator;
	uint32_t m_totalSize;
	uint8_t* m_head;
	uint8_t* m_tail;
	std::stack< uint8_t* > m_scope;
	std::list< uint8_t* > m_heaps;
};

}

#endif	// traktor_PoolAllocator_H
