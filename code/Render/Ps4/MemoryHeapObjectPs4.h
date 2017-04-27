/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_MemoryHeapObjectPs4_H
#define traktor_render_MemoryHeapObjectPs4_H

#include "Core/Config.h"

namespace traktor
{
	namespace render
	{

class MemoryHeapPs4;

class MemoryHeapObjectPs4
{
public:
	MemoryHeapObjectPs4();

	void free();

	size_t getAlignment() const { return m_alignment; }

	size_t getSize() const { return m_size; }

	void* getPointer() const { return m_pointer; }

private:
	friend class MemoryHeapPs4;

	bool m_immutable;
	size_t m_alignment;
	size_t m_size;
	void* m_pointer;
	MemoryHeapPs4* m_heap;
};

	}
}

#endif	// traktor_render_MemoryHeapObjectPs4_H
