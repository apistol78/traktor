/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Thread/Acquire.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/SharedBufferHeapDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SharedBufferHeapDx11", SharedBufferHeapDx11, Object)

SharedBufferHeapDx11::SharedBufferHeapDx11(ContextDx11* context, const D3D11_BUFFER_DESC& dbd)
:	m_context(context)
,	m_dbd(dbd)
{
}

void SharedBufferHeapDx11::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::map< uint32_t, std::list< Chain > >::iterator i = m_chains.begin(); i != m_chains.end(); ++i)
	{
		for (std::list< Chain >::iterator j = i->second.begin(); j != i->second.end(); ++j)
			m_context->releaseComRef(j->d3dBuffer);
	}	
	m_chains.clear();
}

bool SharedBufferHeapDx11::alloc(uint32_t bufferSize, uint32_t vertexStride, Chunk& outChunk)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	Chain chain;
	HRESULT hr;

	std::list< Chain >& chains = m_chains[vertexStride];

	// Allocate chunk from any chain which contain a free block large enough.
	for (std::list< Chain >::iterator i = chains.begin(); i != chains.end(); ++i)
	{
		for (std::vector< FreeList >::iterator j = i->freeList.begin(); j != i->freeList.end(); ++j)
		{
			// Align up offset onto stride boundaries as "vertex base offset" used in Draw...
			// methods are vertex index and not byte offsets.
			uint32_t vertexOffset = alignUp(j->offset, vertexStride);
			T_FATAL_ASSERT (vertexOffset % vertexStride == 0);

			uint32_t allocSize = bufferSize + (vertexOffset - j->offset);
			if (j->size >= allocSize)
			{
				outChunk.d3dBuffer = i->d3dBuffer;
				outChunk.byteOffset = j->offset;
				outChunk.vertexOffset = vertexOffset;
				outChunk.size = allocSize;

				if (allocSize < j->size)
				{
					j->size -= allocSize;
					j->offset += allocSize;
				}
				else
					i->freeList.erase(j);

				return true;
			}
		}
	}

	// Prepare buffer descriptor; if requested size is larger than configured we
	// create one such large buffer instead.
	D3D11_BUFFER_DESC dbd = m_dbd;
	dbd.ByteWidth = max(dbd.ByteWidth, bufferSize);

	// No free block found; allocate a new chain.
	hr = m_context->getD3DDevice()->CreateBuffer(&dbd, NULL, &chain.d3dBuffer.getAssign());
	if (FAILED(hr))
		return false;

	// Add a free list entry of what is left in chain after this allocation.
	FreeList fl;
	fl.offset = bufferSize;
	fl.size = m_dbd.ByteWidth - bufferSize;
	if (fl.size > 0)
		chain.freeList.push_back(fl);

	outChunk.d3dBuffer = chain.d3dBuffer;
	outChunk.byteOffset = 0;
	outChunk.vertexOffset = 0;
	outChunk.size = bufferSize;

	chains.push_back(chain);
	return true;
}

void SharedBufferHeapDx11::free(Chunk& chunk)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::map< uint32_t, std::list< Chain > >::iterator i = m_chains.begin(); i != m_chains.end(); ++i)
	{
		for (std::list< Chain >::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if (j->d3dBuffer == chunk.d3dBuffer)
			{
				// Add new free entry to chain.
				FreeList fl;
				fl.offset = chunk.byteOffset;
				fl.size = chunk.size;
				j->freeList.push_back(fl);

				// Sort free entries.
				std::sort(j->freeList.begin(), j->freeList.end());

				// Merge free entries.
				for (size_t k = 0; k < j->freeList.size() - 1; )
				{
					FreeList& fl_0 = j->freeList[k];
					FreeList& fl_1 = j->freeList[k + 1];

					if (fl_0.offset + fl_0.size == fl_1.offset)
					{
						fl_0.size += fl_1.size;
						j->freeList.erase(j->freeList.begin() + k + 1);
					}
					else
						++k;
				}

				return;
			}
		}
	}
}

bool SharedBufferHeapDx11::FreeList::operator < (const FreeList& rh) const
{
	return offset + size <= rh.offset;
}

	}
}
