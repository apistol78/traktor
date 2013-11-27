#include "Core/Thread/Acquire.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/IndexBufferDynamicDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferDynamicDx11", IndexBufferDynamicDx11, IndexBufferDx11)

Ref< IndexBufferDynamicDx11 > IndexBufferDynamicDx11::create(ContextDx11* context, IndexType indexType, uint32_t bufferSize)
{
	ComRef< ID3D11DeviceContext > d3dDeferredContext;
	ComRef< ID3D11Buffer > d3dBuffer;
	D3D11_BUFFER_DESC dbd;
	HRESULT hr;

	hr = context->getD3DDevice()->CreateDeferredContext(0, &d3dDeferredContext.getAssign());
	if (FAILED(hr))
		return 0;

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
	ib->m_d3dDeferredContext = d3dDeferredContext;
	ib->m_d3dBuffer = d3dBuffer;

	return ib;
}

IndexBufferDynamicDx11::~IndexBufferDynamicDx11()
{
	destroy();
}

void IndexBufferDynamicDx11::destroy()
{
	if (!m_context)
		return;

	m_context->releaseComRef(m_d3dBuffer);
	m_context->releaseComRef(m_d3dDeferredContext);
	m_context = 0;
}

void* IndexBufferDynamicDx11::lock()
{
	D3D11_MAPPED_SUBRESOURCE dm;
	HRESULT hr;

	hr = m_d3dDeferredContext->Map(m_d3dBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dm);
	if (FAILED(hr))
		return 0;

	return dm.pData;
}

void IndexBufferDynamicDx11::unlock()
{
	m_d3dDeferredContext->Unmap(m_d3dBuffer, 0);
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_d3dDeferredContext->FinishCommandList(FALSE, &m_d3dPendingCommandList.getAssign());
	}
}

void IndexBufferDynamicDx11::prepare(ID3D11DeviceContext* d3dDeviceContext, StateCache& stateCache)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_d3dPendingCommandList)
	{
		d3dDeviceContext->ExecuteCommandList(m_d3dPendingCommandList, TRUE);
		m_d3dPendingCommandList.release();
	}
	IndexBufferDx11::prepare(d3dDeviceContext, stateCache);
}

IndexBufferDynamicDx11::IndexBufferDynamicDx11(IndexType indexType, uint32_t bufferSize)
:	IndexBufferDx11(indexType, bufferSize)
{
}

	}
}
