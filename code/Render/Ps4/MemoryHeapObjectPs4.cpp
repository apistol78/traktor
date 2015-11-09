#include "Render/Ps4/MemoryHeapPs4.h"
#include "Render/Ps4/MemoryHeapObjectPs4.h"

namespace traktor
{
	namespace render
	{

MemoryHeapObjectPs4::MemoryHeapObjectPs4()
:	m_immutable(false)
,	m_alignment(0)
,	m_size(0)
,	m_pointer(0)
,	m_heap(0)
{
}

void MemoryHeapObjectPs4::free()
{
	T_FATAL_ASSERT_M (m_heap, L"No heap; already freed?");
	m_heap->free(this);
}

	}
}
