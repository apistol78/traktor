/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Thread/Acquire.h"
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
	IBufferHeapDx11* bufferHeap,
	uint32_t bufferSize,
	const AlignedVector< VertexElement >& vertexElements
)
{
	IBufferHeapDx11::Chunk bufferChunk;

	T_FATAL_ASSERT (bufferSize > 0);

	uint32_t vertexStride = getVertexSize(vertexElements);
	T_FATAL_ASSERT (vertexStride > 0);

	if (!bufferHeap->alloc(bufferSize, vertexStride, bufferChunk))
		return 0;

	T_FATAL_ASSERT (bufferChunk.vertexOffset % vertexStride == 0);

	Ref< VertexBufferStaticDx11 > vb = new VertexBufferStaticDx11(bufferSize);

	vb->m_context = context;
	vb->m_bufferHeap = bufferHeap;
	vb->m_bufferChunk = bufferChunk;
	vb->m_d3dBuffer = bufferChunk.d3dBuffer;
	vb->m_d3dStride = vertexStride;
	vb->m_d3dBaseVertexOffset = bufferChunk.vertexOffset / vertexStride;

	vb->m_d3dInputElements.resize(vertexElements.size());
	for (uint32_t i = 0; i < vertexElements.size(); ++i)
	{
		T_FATAL_ASSERT (vertexElements[i].getDataUsage() < sizeof_array(c_dxgiInputSemantic));
		T_FATAL_ASSERT (vertexElements[i].getDataType() < sizeof_array(c_dxgiInputType));

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
	if (m_context)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
		if (m_bufferHeap)
		{
			m_bufferHeap->free(m_bufferChunk);
			m_bufferHeap = 0;
		}
		m_context->releaseComRef(m_d3dBuffer);
	}
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

void VertexBufferStaticDx11::prepare(ID3D11DeviceContext* d3dDeviceContext, StateCache& stateCache)
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
	VertexBufferDx11::prepare(d3dDeviceContext, stateCache);
}

VertexBufferStaticDx11::VertexBufferStaticDx11(uint32_t bufferSize)
:	VertexBufferDx11(bufferSize)
{
}

	}
}
