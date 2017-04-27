/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_BlockAllocator_H
#define traktor_BlockAllocator_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Fixed size block allocator.
 * \ingroup Core
 *
 * Simple fixed size block allocator with O(1) complexity
 * when allocating and freeing blocks.
 */
class T_DLLCLASS BlockAllocator
{
public:
	BlockAllocator(void* top, int count, size_t size);
	
	void* top();
	
	void* alloc();
	
	bool free(void* p);

	bool belong(const void* p) const;
	
private:
	size_t* m_top;
	size_t* m_end;
	size_t* m_free;
#if defined (_DEBUG)
	size_t m_alloced;
	size_t m_size;
	bool m_full;
#endif
};

}

#endif	// traktor_BlockAllocator_H
