#include "Render/Ps3/LocalMemoryAllocator.h"
#include "Core/Thread/CriticalSection.h"
#include "Core/Thread/Acquire.h"
#include "Core/Log/Log.h"

#define T_LOCAL_HEAP_THREAD_SAFE

namespace traktor
{
	namespace render
	{
		namespace
		{

#if defined(T_LOCAL_HEAP_THREAD_SAFE)
CriticalSection _heapLock;
#	define HEAP_LOCK Acquire< CriticalSection > __prvlock__(_heapLock);
#else
#	define HEAP_LOCK
#endif

		}

LocalMemoryAllocator& LocalMemoryAllocator::getInstance()
{
	static LocalMemoryAllocator instance;
	return instance;
}

void LocalMemoryAllocator::setHeap(void* heapTop, size_t heapSize)
{
	HEAP_LOCK

	m_heapTop = (unsigned char*)heapTop;
	m_allocated = 0;
	m_size = heapSize;

	Chunk chunk = { false, heapSize };
	m_chunks.push_back(chunk);
}

void* LocalMemoryAllocator::alloc(const size_t size)
{
	HEAP_LOCK

	void* ptr = 0;

	size_t offset = 0;
	for (std::list< Chunk >::iterator i = m_chunks.begin(); i != m_chunks.end(); ++i)
	{
		if (!i->allocated && i->size >= size)
		{
			if (i->size > size)
			{
				i->size -= size;

				Chunk chunk = { false, size };
				i = m_chunks.insert(i, chunk);
			}

			ptr = &m_heapTop[offset];
			i->allocated = true;

			m_allocated += size;
			break;
		}
		offset += i->size;
	}

	assert (ptr);
	return ptr;
}

void* LocalMemoryAllocator::allocAlign(const size_t size, const size_t align)
{
	HEAP_LOCK

	size_t base = (size_t)alloc(size + align);
	base = (base + align) & ~(align - 1);

	return (void*)base;
}

void LocalMemoryAllocator::free(void* ptr)
{
	HEAP_LOCK

	size_t offset = 0;
	for (std::list< Chunk >::iterator i = m_chunks.begin(); i != m_chunks.end(); ++i)
	{
		if (i->allocated && ptr >= &m_heapTop[offset] && ptr <= &m_heapTop[offset + i->size])
		{
			i->allocated = false;
			m_allocated -= i->size;
			break;
		}
		offset += i->size;
	}

	for (std::list< Chunk >::iterator i = m_chunks.begin(); ; )
	{
		std::list< Chunk >::iterator j = i; ++j;
		if (j == m_chunks.end())
			break;

		if (!i->allocated && !j->allocated)
		{
			i->size += j->size;
			m_chunks.erase(j);
		}
		else
			++i;
	}
}

LocalMemoryAllocator::LocalMemoryAllocator()
:	m_heapTop(0)
{
}

void LocalMemoryAllocator::dumpChunks()
{
	size_t offset = 0;
	log::info << "Local memory heap, " << m_chunks.size() << " chunks" << Endl;
	log::info << m_allocated << " allocated of " << m_size << " (" << (m_size - m_allocated) << " free)" << Endl;
	log::info << IncreaseIndent;
	for (std::list< Chunk >::iterator i = m_chunks.begin(); i != m_chunks.end(); ++i)
	{
		log::info << "Chunk, offset = " << offset << ", size = " << i->size << ", " << (i->allocated ? "allocated" : "free") << Endl;
		offset += i->size;
	}
	log::info << DecreaseIndent;
}

	}
}
