#include "Core/Misc/SafeDestroy.h"
#include "Render/Vrfy/IndexBufferVrfy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/ResourceTracker.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferVrfy", IndexBufferVrfy, IndexBuffer)

IndexBufferVrfy::IndexBufferVrfy(ResourceTracker* resourceTracker, IndexBuffer* indexBuffer, IndexType indexType, uint32_t bufferSize)
:	IndexBuffer(indexType, bufferSize)
,	m_resourceTracker(resourceTracker)
,	m_indexBuffer(indexBuffer)
,	m_locked(false)
{
	m_resourceTracker->add(this);
}

IndexBufferVrfy::~IndexBufferVrfy()
{
	m_resourceTracker->remove(this);
}

void IndexBufferVrfy::destroy()
{
	T_CAPTURE_ASSERT (m_indexBuffer, L"Index buffer already destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Cannot destroy locked index buffer.");
	safeDestroy(m_indexBuffer);
}

void* IndexBufferVrfy::lock()
{
	T_CAPTURE_ASSERT (m_indexBuffer, L"Index buffer destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Index buffer already locked.");

	if (!m_indexBuffer)
		return 0;

	void* p = m_indexBuffer->lock();
	if (p)
		m_locked = true;

	return p;
}

void IndexBufferVrfy::unlock()
{
	T_CAPTURE_ASSERT (m_indexBuffer, L"Index buffer destroyed.");
	T_CAPTURE_ASSERT (m_locked, L"Index buffer not locked.");

	if (!m_indexBuffer)
		return;

	m_indexBuffer->unlock();
	m_locked = false;
}

const IBufferView* IndexBufferVrfy::getBufferView() const
{
	T_CAPTURE_ASSERT (m_indexBuffer, L"Index buffer destroyed.");

	if (!m_indexBuffer)
		return nullptr;

	return m_indexBuffer->getBufferView();
}

	}
}
