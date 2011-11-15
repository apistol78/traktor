#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/IndexBufferDynamicDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferDynamicDx11", IndexBufferDynamicDx11, IndexBufferDx11)

Ref< IndexBufferDynamicDx11 > IndexBufferDynamicDx11::create(ContextDx11* context, IndexType indexType, uint32_t bufferSize)
{
	ComRef< ID3D11Buffer > d3dBuffer;
	D3D11_BUFFER_DESC dbd;
	HRESULT hr;

	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D11_USAGE_DYNAMIC;
	dbd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dbd.MiscFlags = 0;

	hr = context->getD3DDevice()->CreateBuffer(&dbd, NULL, &d3dBuffer.getAssign());
	if (FAILED(hr))
		return 0;

	Ref< IndexBufferDynamicDx11 > ib = new IndexBufferDynamicDx11(indexType, bufferSize);

	ib->m_context = context;
	ib->m_d3dBuffer = d3dBuffer;
	ib->m_locked = false;

	return ib;
}

IndexBufferDynamicDx11::~IndexBufferDynamicDx11()
{
	destroy();
}

void IndexBufferDynamicDx11::destroy()
{
	T_ASSERT (!m_locked);
	
	if (!m_context)
		return;

	m_context->releaseComRef(m_d3dBuffer);
	m_context = 0;
}

void* IndexBufferDynamicDx11::lock()
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

void IndexBufferDynamicDx11::unlock()
{
	T_ASSERT (m_d3dBuffer);
	T_ASSERT (m_locked);

	m_context->getD3DDeviceContext()->Unmap(m_d3dBuffer, 0);
	m_locked = false;
}

IndexBufferDynamicDx11::IndexBufferDynamicDx11(IndexType indexType, uint32_t bufferSize)
:	IndexBufferDx11(indexType, bufferSize)
,	m_locked(false)
{
}

	}
}
