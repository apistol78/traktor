/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_LocalMemoryObject_H
#define traktor_render_LocalMemoryObject_H

#include "Core/Config.h"

namespace traktor
{
	namespace render
	{

class MemoryHeap;

class MemoryHeapObject
{
public:
	MemoryHeapObject();

	void free();

	size_t getAlignment() const { return m_alignment; }

	size_t getSize() const { return m_size; }

	void* getPointer() const { return m_pointer; }

	uint32_t getOffset() const { return m_offset; }

	uint8_t getLocation() const { return m_location; }

private:
	friend class MemoryHeap;

	bool m_immutable;
	size_t m_alignment;
	size_t m_size;
	void* m_pointer;
	uint32_t m_offset;
	uint8_t m_location;
	MemoryHeap* m_heap;
};

	}
}

#endif	// traktor_render_LocalMemoryObject_H
