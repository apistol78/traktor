#ifndef traktor_render_LocalMemoryAllocator_H
#define traktor_render_LocalMemoryAllocator_H

#include <cstddef>
#include <list>

namespace traktor
{
	namespace render
	{

class LocalMemoryAllocator
{
public:
	static LocalMemoryAllocator& getInstance();

	void setHeap(void* heapTop, size_t heapSize);

	void* alloc(const size_t size);

	void* allocAlign(const size_t size, const size_t align);

	void free(void* ptr);

private:
	struct Chunk
	{
		bool allocated;
		size_t size;
	};

	unsigned char* m_heapTop;
	std::list< Chunk > m_chunks;
	size_t m_allocated;
	size_t m_size;

	LocalMemoryAllocator();

	void dumpChunks();
};

	}
}

#endif	// traktor_render_LocalMemoryAllocator_H
