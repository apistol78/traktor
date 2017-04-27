/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_MemoryHeapPs4_H
#define traktor_render_MemoryHeapPs4_H

#include <vector>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace render
	{

class MemoryHeapObjectPs4;

class MemoryHeapPs4 : public Object
{
public:
	MemoryHeapPs4(void* heap, size_t heapSize);

	MemoryHeapObjectPs4* alloc(size_t size, size_t align, bool immutable);

	size_t getAvailable() const;

	size_t getObjectCount() const;

	void compact();

private:
	friend class MemoryHeapObjectPs4;

	mutable Semaphore m_lock;
	uint8_t* m_heap;
	size_t m_heapSize;
	std::vector< MemoryHeapObjectPs4* > m_objects;		//< Sorted list of alive objects.
	bool m_shouldCompact;

	void free(MemoryHeapObjectPs4* object);
};

	}
}

#endif	// traktor_render_MemoryHeapPs4_H
