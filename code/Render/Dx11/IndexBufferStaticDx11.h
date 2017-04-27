/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IndexBufferStaticDx11_H
#define traktor_render_IndexBufferStaticDx11_H

#include "Core/Misc/AutoPtr.h"
#include "Render/Dx11/IBufferHeapDx11.h"
#include "Render/Dx11/IndexBufferDx11.h"

namespace traktor
{
	namespace render
	{

class IBufferHeapDx11;
class ContextDx11;

/*!
 * \ingroup DX11
 */
class IndexBufferStaticDx11 : public IndexBufferDx11
{
	T_RTTI_CLASS;

public:
	static Ref< IndexBufferStaticDx11 > create(ContextDx11* context, IBufferHeapDx11* bufferHeap, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferStaticDx11();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void prepare(ID3D11DeviceContext* d3dDeviceContext, StateCache& stateCache) T_OVERRIDE T_FINAL;

private:
	Ref< ContextDx11 > m_context;
	Ref< IBufferHeapDx11 > m_bufferHeap;
	IBufferHeapDx11::Chunk m_bufferChunk;
	AutoArrayPtr< uint8_t > m_data;

	IndexBufferStaticDx11(IndexType indexType, uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_IndexBufferStaticDx11_H
