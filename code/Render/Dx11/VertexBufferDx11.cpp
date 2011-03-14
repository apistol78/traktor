#include "Render/Dx11/Platform.h"
#include "Render/Dx11/VertexBufferDx11.h"
#include "Render/Dx11/ContextDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDx11", VertexBufferDx11, VertexBuffer)

VertexBufferDx11::VertexBufferDx11(
	ContextDx11* context,
	uint32_t bufferSize,
	ID3D11Buffer* d3dBuffer,
	UINT d3dStride,
	const std::vector< D3D11_INPUT_ELEMENT_DESC >& d3dInputElements
)
:	VertexBuffer(bufferSize)
,	m_context(context)
,	m_d3dBuffer(d3dBuffer)
,	m_d3dStride(d3dStride)
,	m_d3dInputElements(d3dInputElements)
,	m_locked(false)
{
}

VertexBufferDx11::~VertexBufferDx11()
{
	destroy();
}

void VertexBufferDx11::destroy()
{
	T_ASSERT (!m_locked);
	
	if (!m_context)
		return;

	m_context->releaseComRef(m_d3dBuffer);
	m_context = 0;
}

void* VertexBufferDx11::lock()
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

void* VertexBufferDx11::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_ASSERT (!m_locked);
	D3D11_MAPPED_SUBRESOURCE dm;
	HRESULT hr;

	hr = m_context->getD3DDeviceContext()->Map(m_d3dBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dm);
	if (FAILED(hr))
		return 0;

	m_locked = true;
	return static_cast< uint8_t* >(dm.pData) + vertexOffset * m_d3dStride;
}

void VertexBufferDx11::unlock()
{
	T_ASSERT (m_d3dBuffer);
	T_ASSERT (m_locked);
	
	m_context->getD3DDeviceContext()->Unmap(m_d3dBuffer, 0);
	m_locked = false;

	setContentValid(true);
}

const std::vector< D3D11_INPUT_ELEMENT_DESC >& VertexBufferDx11::getD3D11InputElements() const
{
	return m_d3dInputElements;
}

ID3D11Buffer* VertexBufferDx11::getD3D11Buffer() const
{
	return m_d3dBuffer;
}

UINT VertexBufferDx11::getD3D11Stride() const
{
	return m_d3dStride;
}

	}
}
