#include "Core/Misc/Adler32.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/TypesDx11.h"
#include "Render/Dx11/VertexBufferStaticDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferStaticDx11", VertexBufferStaticDx11, VertexBufferDx11)

Ref< VertexBufferStaticDx11 > VertexBufferStaticDx11::create(
	ContextDx11* context,
	uint32_t bufferSize,
	const std::vector< VertexElement >& vertexElements
)
{
	ComRef< ID3D11Buffer > d3dBuffer;
	D3D11_BUFFER_DESC dbd;
	HRESULT hr;

	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D11_USAGE_DEFAULT;
	dbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	dbd.CPUAccessFlags = 0;
	dbd.MiscFlags = 0;

	hr = context->getD3DDevice()->CreateBuffer(&dbd, NULL, &d3dBuffer.getAssign());
	if (FAILED(hr))
		return 0;

	Ref< VertexBufferStaticDx11 > vb = new VertexBufferStaticDx11(bufferSize);

	vb->m_context = context;
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

VertexBufferStaticDx11::~VertexBufferStaticDx11()
{
	destroy();
}

void VertexBufferStaticDx11::destroy()
{
	if (!m_context)
		return;

	m_context->releaseComRef(m_d3dBuffer);
	m_context = 0;
}

void* VertexBufferStaticDx11::lock()
{
	T_ASSERT (!m_data.ptr());
	m_data.reset(new uint8_t [getBufferSize()]);
	return m_data.ptr();
}

void* VertexBufferStaticDx11::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ERROR;
	return 0;
}

void VertexBufferStaticDx11::unlock()
{
	setContentValid(true);
}

void VertexBufferStaticDx11::prepare(ID3D11DeviceContext* d3dDeviceContext)
{
	if (m_data.ptr())
	{
		d3dDeviceContext->UpdateSubresource(
			m_d3dBuffer,
			0,
			NULL,
			m_data.c_ptr(),
			getBufferSize(),
			1
		);
		m_data.release();
	}
	VertexBufferDx11::prepare(d3dDeviceContext);
}

VertexBufferStaticDx11::VertexBufferStaticDx11(uint32_t bufferSize)
:	VertexBufferDx11(bufferSize)
{
}

	}
}
