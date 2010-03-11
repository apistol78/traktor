#ifndef traktor_render_LocalMemoryObject_H
#define traktor_render_LocalMemoryObject_H

#include "Core/Config.h"

namespace traktor
{
	namespace render
	{

class LocalMemoryObject
{
public:
	LocalMemoryObject();

	size_t getSize() const { return m_size; }

	void* getPointer() const { return m_pointer; }

	uint32_t getOffset() const { return m_offset; }

private:
	friend class LocalMemoryManager;

	bool m_immutable;
	size_t m_alignment;
	size_t m_size;
	void* m_pointer;
	uint32_t m_offset;
};

	}
}

#endif	// traktor_render_LocalMemoryObject_H
