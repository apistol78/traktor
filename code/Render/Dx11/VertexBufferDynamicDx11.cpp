#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
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
	vb->m_data.reset((uint8_t*)Alloc::acquireAlign(bufferSize, 16, T_FILE_LINE));
	vb->m_dirty = false;
	vb->m_d3dBuffer = d3dBuffer;
	vb->m_d3dStride = getVertexSize(vertexElements);

	vb->m_d3dInputElements.resize(vertexElements.size());
	for (uint32_t i = 0; i < vertexElements.size(); ++i)
	{
		T_ASSERT (vertexElements[i].getDataUsage() < sizeof_array(c_dxgiInputSemantic));
		T_ASSERT (vertexElements[i].getDataType() < sizeof_array(c_dxgiInputType));

		std::memset(&vb->m_d3dInputElements[i], 0, sizeof(D3D11_INPUT_ELEMENT_DESC));
		vb->m_d3dInputElements[i].SemanticName = c_dxgiInputSemantic[vertexElements[i].getDataUsage()];
		vb->m_d3dInputElements[i].SemanticIndex = vertexElements[i].getIndex();
		vb->m_d3dInputElements[i].Format = c_dxgiInputType[vertexElements[i].getDataType()];
		vb->m_d3dInputElements[i].InputSlot = 0;
		vb->m_d3dInputElements[i].AlignedByteOffset = vertexElements[i].getOffset();
		vb->m_d3dInputElements[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		vb->m_d3dInputElements[i].InstanceDataStepRate = 0;
	}

	Adler32 hash;
	hash.begin();
	hash.feed(&vb->m_d3dInputElements[0], vb->m_d3dInputElements.size() * sizeof(D3D11_INPUT_ELEMENT_DESC));
	hash.end();

	vb->m_d3dInputElementsHash = hash.get();

	return vb;
}

VertexBufferDynamicDx11::~VertexBufferDynamicDx11()
{
	destroy();
}

void VertexBufferDynamicDx11::destroy()
{
	if (!m_context)
		return;

	m_context->releaseComRef(m_d3dBuffer);
	m_context = 0;
}

void* VertexBufferDynamicDx11::lock()
{
	return m_data.ptr();
}

void* VertexBufferDynamicDx11::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ERROR;
	return 0;
}

void VertexBufferDynamicDx11::unlock()
{
	m_dirty = true;
	setContentValid(true);
}

void VertexBufferDynamicDx11::prepare(ID3D11DeviceContext* d3dDeviceContext)
{
	if (m_dirty)
	{
		D3D11_MAPPED_SUBRESOURCE dm;
		HRESULT hr;

		// See GCN Performance Tip 29: http://developer.amd.com/wordpress/media/2013/05/GCNPerformanceTweets.pdf
		hr = d3dDeviceContext->Map(m_d3dBuffer, 0, /*D3D11_MAP_WRITE_DISCARD*/D3D11_MAP_WRITE_NO_OVERWRITE, 0, &dm);
		if (FAILED(hr))
		{
			T_DEBUG(L"Failed to map dynamic vertex buffer; unable to update");
			return;
		}

		std::memcpy(dm.pData, m_data.c_ptr(), getBufferSize());

		d3dDeviceContext->Unmap(m_d3dBuffer, 0);
		m_dirty = false;
	}
	VertexBufferDx11::prepare(d3dDeviceContext);
}

VertexBufferDynamicDx11::VertexBufferDynamicDx11(uint32_t bufferSize)
:	VertexBufferDx11(bufferSize)
,	m_dirty(false)
{
}

	}
}
