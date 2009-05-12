#include "Render/Ps3/IndexBufferPs3.h"
#include "Render/Ps3/LocalMemoryAllocator.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferPs3", IndexBufferPs3, IndexBuffer)

IndexBufferPs3::IndexBufferPs3(void* ptr, uint32_t offset, IndexType indexType, int bufferSize)
:	IndexBuffer(indexType, bufferSize)
,	m_ptr(ptr)
,	m_offset(offset)
{
}

IndexBufferPs3::~IndexBufferPs3()
{
	destroy();
}

void IndexBufferPs3::destroy()
{
	if (m_ptr)
	{
		LocalMemoryAllocator::getInstance().free(m_ptr);

		m_ptr = 0;
		m_offset = 0;
	}
}

void* IndexBufferPs3::lock()
{
	return m_ptr;
}

void IndexBufferPs3::unlock()
{
}

	}
}
