#include "Core/Misc/SafeDestroy.h"
#include "Render/Capture/IndexBufferCapture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferCapture", IndexBufferCapture, IndexBuffer)

IndexBufferCapture::IndexBufferCapture(IndexBuffer* indexBuffer, IndexType indexType, uint32_t bufferSize)
:	IndexBuffer(indexType, bufferSize)
,	m_indexBuffer(indexBuffer)
,	m_locked(false)
{
}

void IndexBufferCapture::destroy()
{
	T_FATAL_ASSERT_M (m_indexBuffer, L"Render error: Index buffer already destroyed.");
	T_FATAL_ASSERT_M (!m_locked, L"Render error: Cannot destroy locked index buffer.");
	safeDestroy(m_indexBuffer);
}

void* IndexBufferCapture::lock()
{
	T_FATAL_ASSERT_M (m_indexBuffer, L"Render error: Index buffer destroyed.");
	T_FATAL_ASSERT_M (!m_locked, L"Render error: Index buffer already locked.");
	void* p = m_indexBuffer->lock();
	if (p)
		m_locked = true;
	return p;
}
	
void IndexBufferCapture::unlock()
{
	T_FATAL_ASSERT_M (m_indexBuffer, L"Render error: Index buffer destroyed.");
	T_FATAL_ASSERT_M (m_locked, L"Render error: Index buffer not locked.");
	m_indexBuffer->unlock();
	m_locked = false;
}

	}
}
