#include "Render/Ps3/IndexBufferPs3.h"
#include "Render/Ps3/MemoryHeapObject.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferPs3", IndexBufferPs3, IndexBuffer)

IndexBufferPs3::IndexBufferPs3(MemoryHeapObject* ibo, IndexType indexType, int bufferSize)
:	IndexBuffer(indexType, bufferSize)
,	m_ibo(ibo)
{
}

IndexBufferPs3::~IndexBufferPs3()
{
	destroy();
}

void IndexBufferPs3::destroy()
{
	if (m_ibo)
	{
		m_ibo->free();
		m_ibo = 0;
	}
}

void* IndexBufferPs3::lock()
{
	return m_ibo->getPointer();
}

void IndexBufferPs3::unlock()
{
}

uint8_t IndexBufferPs3::getLocation() const
{
	return m_ibo->getLocation();
}

uint32_t IndexBufferPs3::getOffset() const
{
	return m_ibo->getOffset();
}

	}
}
