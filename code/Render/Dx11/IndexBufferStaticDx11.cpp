#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/IndexBufferStaticDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferStaticDx11", IndexBufferStaticDx11, IndexBufferDx11)

Ref< IndexBufferStaticDx11 > IndexBufferStaticDx11::create(ContextDx11* context, IndexType indexType, uint32_t bufferSize)
{
	ComRef< ID3D11Buffer > d3dBuffer;
	D3D11_BUFFER_DESC dbd;
	HRESULT hr;

	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D11_USAGE_DEFAULT;
	dbd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	dbd.CPUAccessFlags = 0;
	dbd.MiscFlags = 0;

	hr = context->getD3DDevice()->CreateBuffer(&dbd, NULL, &d3dBuffer.getAssign());
	if (FAILED(hr))
		return 0;

	Ref< IndexBufferStaticDx11 > ib = new IndexBufferStaticDx11(indexType, bufferSize);

	ib->m_context = context;
	ib->m_d3dBuffer = d3dBuffer;

	return ib;
}

IndexBufferStaticDx11::~IndexBufferStaticDx11()
{
	destroy();
}

void IndexBufferStaticDx11::destroy()
{
	if (!m_context)
		return;

	m_context->releaseComRef(m_d3dBuffer);
	m_context = 0;
}

void* IndexBufferStaticDx11::lock()
{
	T_ASSERT (!m_data.ptr());
	m_data.reset(new uint8_t [getBufferSize()]);
	return m_data.ptr();
}

void IndexBufferStaticDx11::unlock()
{
}

void IndexBufferStaticDx11::prepare(ID3D11DeviceContext* d3dDeviceContext, StateCache& stateCache)
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
	IndexBufferDx11::prepare(d3dDeviceContext, stateCache);
}

IndexBufferStaticDx11::IndexBufferStaticDx11(IndexType indexType, uint32_t bufferSize)
:	IndexBufferDx11(indexType, bufferSize)
{
}

	}
}
