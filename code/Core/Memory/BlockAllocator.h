#pragma once

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

/*! Fixed size block allocator.
 * \ingroup Core
 *
 * Simple fixed size block allocator with O(1) complexity
 * when allocating and freeing blocks.
 */
class T_DLLCLASS BlockAllocator
{
public:
	explicit BlockAllocator(void* top, int32_t count, size_t size);

	void* top() { return m_top; }

	void* alloc();

	bool free(void* p);

	bool belong(const void* p) const { return (p >= m_top && p < m_end); }

private:
	intptr_t* m_top;
	intptr_t* m_end;
	intptr_t* m_free;
#if defined (_DEBUG)
	size_t m_alloced;
	size_t m_size;
	bool m_full;
#endif
};

}

