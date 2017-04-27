/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Ps3/MemoryHeap.h"
#include "Render/Ps3/MemoryHeapObject.h"

namespace traktor
{
	namespace render
	{

MemoryHeapObject::MemoryHeapObject()
:	m_immutable(false)
,	m_alignment(0)
,	m_size(0)
,	m_pointer(0)
,	m_offset(0)
,	m_location(0)
,	m_heap(0)
{
}

void MemoryHeapObject::free()
{
	T_FATAL_ASSERT_M (m_heap, L"No heap; already freed?");
	m_heap->free(this);
}

	}
}
