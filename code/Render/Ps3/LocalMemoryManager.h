#ifndef traktor_render_LocalMemoryManager_H
#define traktor_render_LocalMemoryManager_H

#include <vector>
#include "Core/Config.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace render
	{

class LocalMemoryObject;

class LocalMemoryManager
{
public:
	static LocalMemoryManager& getInstance();

	void setHeap(void* heap, size_t heapSize);

	LocalMemoryObject* alloc(size_t size, size_t align, bool immutable);

	void free(LocalMemoryObject* object);

	size_t available() const;

	void compact();

private:
	mutable Semaphore m_lock;
	uint8_t* m_heap;
	size_t m_heapSize;
	std::vector< LocalMemoryObject* > m_objects;		//< Sorted list of alive objects.
	bool m_shouldCompact;
	uint32_t m_waitLabel;

	LocalMemoryManager();
};

	}
}

#endif	// traktor_render_LocalMemoryManager_H
