#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/TypesDx11.h"
#include "Render/Dx11/VertexBufferDynamicDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDynamicDx11", VertexBufferDynamicDx11, VertexBufferDx11)

Ref< VertexBufferDynamicDx11 > VertexBufferDynamicDx11::create(
	ContextDx11* context,
	uint32_t bufferSize,
	const std::vector< VertexElement >& vertexElements
)
{
	ComRef< ID3D11Buffer > d3dBuffer;
	D3D11_BUFFER_DESC dbd;
	HRESULT hr;

	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D11_USAGE_DYNAMIC;
	dbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dbd.MiscFlags = 0;

	hr = context->getD3DDevice()->CreateBuffer(&dbd, NULL, &d3dBuffer.getAssign());
	if (FAILED(hr))
		return 0;

	Ref< VertexBufferDynamicDx11 > vb = new VertexBufferDynamicDx11(bufferSize);

	vb->m_context = context;
	vb->m_locked = false;
	vb->m_d3dBuffer = d3dBuffer;
	vb->m_d3dStride = getVertexSize(vertexElements);

	vb->m_d3dInputElements.resize(vertexElements.size());
	for (uint32_t i = 0; i < vertexElements.size(); ++i)
	{
		T_ASSERT (vertexElements[i].getDataUsage() < sizeof_array(c_dxgiInputSemantic));
		T_ASSERT (vertexElements[i].getDataType() < sizeof_array(c_dxgiInputType));

		vb->m_d3dInputElements[i].SemanticName = c_dxgiInputSemantic[vertexElements[i].getDataUsage()];
		vb->m_d3dInputElements[i].SemanticIndex = vertexElements[i].getIndex();
		vb->m_d3dInputElements[i].Format = c_dxgiInputType[vertexElements[i].getDataType()];
		vb->m_d3dInputElements[i].InputSlot = 0;
		vb->m_d3dInputElements[i].AlignedByteOffset = vertexElements[i].getOffset();
		vb->m_d3dInputElements[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		vb->m_d3dInputElements[i].InstanceDataStepRate = 0;
	}

	return vb;
}

VertexBufferDynamicDx11::~VertexBufferDynamicDx11()
{
	destroy();
}

void VertexBufferDynamicDx11::destroy()
{
	T_ASSERT (!m_locked);
	
	if (!m_context)
		return;

	m_context->releaseComRef(m_d3dBuffer);
	m_context = 0;
}

void* VertexBufferDynamicDx11::lock()
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

void* VertexBufferDynamicDx11::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_ASSERT (!m_locked);
	D3D11_MAPPED_SUBRESOURCE dm;
	HRESULT hr;

	hr = m_context->getD3DDeviceContext()->Map(m_d3dBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &dm);
	if (FAILED(hr))
		return 0;

	m_locked = true;
	return static_cast< uint8_t* >(dm.pData) + vertexOffset * m_d3dStride;
}

void VertexBufferDynamicDx11::unlock()
{
	T_ASSERT (m_d3dBuffer);
	T_ASSERT (m_locked);
	
	m_context->getD3DDeviceContext()->Unmap(m_d3dBuffer, 0);
	m_locked = false;

	setContentValid(true);
}

void VertexBufferDynamicDx11::prepare()
{
}

VertexBufferDynamicDx11::VertexBufferDynamicDx11(uint32_t bufferSize)
:	VertexBufferDx11(bufferSize)
,	m_locked(false)
{
}

	}
}
