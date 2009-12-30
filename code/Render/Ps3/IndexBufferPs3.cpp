#include "Render/Ps3/IndexBufferPs3.h"
#include "Render/Ps3/LocalMemoryManager.h"
#include "Render/Ps3/LocalMemoryObject.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferPs3", IndexBufferPs3, IndexBuffer)

IndexBufferPs3::IndexBufferPs3(LocalMemoryObject* ibo, IndexType indexType, int bufferSize)
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
		LocalMemoryManager::getInstance().free(m_ibo);
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

uint32_t IndexBufferPs3::getOffset() const
{
	return m_ibo->getOffset();
}

	}
}
