#include "Render/Dx10/Platform.h"
#include "Render/Dx10/IndexBufferDx10.h"
#include "Render/Dx10/ContextDx10.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferDx10", IndexBufferDx10, IndexBuffer)

IndexBufferDx10::IndexBufferDx10(ContextDx10* context, IndexType indexType, uint32_t bufferSize, ID3D10Buffer* d3dBuffer)
:	IndexBuffer(indexType, bufferSize)
,	m_context(context)
,	m_d3dBuffer(d3dBuffer)
,	m_locked(false)
{
}

IndexBufferDx10::~IndexBufferDx10()
{
	destroy();
}

void IndexBufferDx10::destroy()
{
	T_ASSERT (!m_locked);
	if (m_context && m_d3dBuffer)
		m_context->releaseComRef(m_d3dBuffer);
}

void* IndexBufferDx10::lock()
{
	T_ASSERT (!m_locked);
	void* mapped;
	HRESULT hr;

	hr = m_d3dBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr))
		return 0;

	m_locked = true;
	return mapped;
}

void IndexBufferDx10::unlock()
{
	T_ASSERT (m_d3dBuffer);
	T_ASSERT (m_locked);
	m_d3dBuffer->Unmap();
	m_locked = false;
}

ID3D10Buffer* IndexBufferDx10::getD3D10Buffer() const
{
	return m_d3dBuffer;
}

	}
}
