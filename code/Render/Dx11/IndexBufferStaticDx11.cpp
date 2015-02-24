#include "Core/Thread/Acquire.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/IndexBufferStaticDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferStaticDx11", IndexBufferStaticDx11, IndexBufferDx11)

Ref< IndexBufferStaticDx11 > IndexBufferStaticDx11::create(ContextDx11* context, IBufferHeapDx11* bufferHeap, IndexType indexType, uint32_t bufferSize)
{
	IBufferHeapDx11::Chunk bufferChunk;

	uint32_t indexStride = (indexType == ItUInt16) ? 2 : 4;

	if (!bufferHeap->alloc(bufferSize, indexStride, bufferChunk))
		return 0;

	Ref< IndexBufferStaticDx11 > ib = new IndexBufferStaticDx11(indexType, bufferSize);

	ib->m_context = context;
	ib->m_bufferHeap = bufferHeap;
	ib->m_bufferChunk = bufferChunk;
	ib->m_d3dBuffer = bufferChunk.d3dBuffer;
	ib->m_d3dBaseIndexOffset = bufferChunk.vertexOffset / indexStride;

	return ib;
}

IndexBufferStaticDx11::~IndexBufferStaticDx11()
{
	destroy();
}

void IndexBufferStaticDx11::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	if (m_bufferHeap)
	{
		m_bufferHeap->free(m_bufferChunk);
		m_bufferHeap = 0;
	}
	m_context->releaseComRef(m_d3dBuffer);
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
		D3D11_BOX d3db;
		
		d3db.left = m_bufferChunk.vertexOffset;
		d3db.right = m_bufferChunk.vertexOffset + getBufferSize();
		d3db.top = 0;
		d3db.bottom = 1;
		d3db.front = 0;
		d3db.back = 1;

		d3dDeviceContext->UpdateSubresource(
			m_d3dBuffer,
			0,
			&d3db,
			m_data.c_ptr(),
			0,
			0
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
