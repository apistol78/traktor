#include "Render/Dx11/Platform.h"
#include "Render/Dx11/IndexBufferDx11.h"
#include "Render/Dx11/ContextDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferDx11", IndexBufferDx11, IndexBuffer)

IndexBufferDx11::IndexBufferDx11(ContextDx11* context, IndexType indexType, uint32_t bufferSize, ID3D11Buffer* d3dBuffer)
:	IndexBuffer(indexType, bufferSize)
,	m_context(context)
,	m_d3dBuffer(d3dBuffer)
,	m_locked(false)
{
}

IndexBufferDx11::~IndexBufferDx11()
{
	destroy();
}

void IndexBufferDx11::destroy()
{
	T_ASSERT (!m_locked);
	
	if (!m_context)
		return;

	m_context->releaseComRef(m_d3dBuffer);
	m_context = 0;
}

void* IndexBufferDx11::lock()
{
	T_ASSERT (!m_locked);
	D3D11_MAPPED_SUBRESOURCE dm;
	HRESULT hr;

	hr = m_context->getD3DDeviceContext()->Map(m_d3dBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dm);
	if (FAILED(hr))
		return 0;

	m_locked = true;
	return dm.pData;
}

void IndexBufferDx11::unlock()
{
	T_ASSERT (m_d3dBuffer);
	T_ASSERT (m_locked);

	m_context->getD3DDeviceContext()->Unmap(m_d3dBuffer, 0);
	m_locked = false;
}

ID3D11Buffer* IndexBufferDx11::getD3D11Buffer() const
{
	return m_d3dBuffer;
}

	}
}
