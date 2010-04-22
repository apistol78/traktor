#include "Render/Dx10/Platform.h"
#include "Render/Dx10/VertexBufferDx10.h"
#include "Render/Dx10/ContextDx10.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDx10", VertexBufferDx10, VertexBuffer)

VertexBufferDx10::VertexBufferDx10(
	ContextDx10* context,
	uint32_t bufferSize,
	ID3D10Buffer* d3dBuffer,
	UINT d3dStride,
	const std::vector< D3D10_INPUT_ELEMENT_DESC >& d3dInputElements
)
:	VertexBuffer(bufferSize)
,	m_context(context)
,	m_d3dBuffer(d3dBuffer)
,	m_d3dStride(d3dStride)
,	m_d3dInputElements(d3dInputElements)
,	m_locked(false)
{
}

VertexBufferDx10::~VertexBufferDx10()
{
	destroy();
}

void VertexBufferDx10::destroy()
{
	T_ASSERT (!m_locked);
	if (m_context && m_d3dBuffer)
		m_context->releaseComRef(m_d3dBuffer);
}

void* VertexBufferDx10::lock()
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

void* VertexBufferDx10::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_ASSERT (!m_locked);
	void* mapped;
	HRESULT hr;

	hr = m_d3dBuffer->Map(D3D10_MAP_WRITE_NO_OVERWRITE, 0, &mapped);
	if (FAILED(hr))
		return 0;

	m_locked = true;
	return static_cast< uint8_t* >(mapped) + vertexOffset * m_d3dStride;
}

void VertexBufferDx10::unlock()
{
	T_ASSERT (m_d3dBuffer);
	T_ASSERT (m_locked);
	
	m_d3dBuffer->Unmap();
	m_locked = false;

	setContentValid(true);
}

const std::vector< D3D10_INPUT_ELEMENT_DESC >& VertexBufferDx10::getD3D10InputElements() const
{
	return m_d3dInputElements;
}

ID3D10Buffer* VertexBufferDx10::getD3D10Buffer() const
{
	return m_d3dBuffer;
}

UINT VertexBufferDx10::getD3D10Stride() const
{
	return m_d3dStride;
}

	}
}
