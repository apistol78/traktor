/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SimpleBufferHeapDx11_H
#define traktor_render_SimpleBufferHeapDx11_H

#include "Render/Dx11/IBufferHeapDx11.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*! \brief Manages DX11 buffer memory.
 * \ingroup DX11
 */
class SimpleBufferHeapDx11 : public IBufferHeapDx11
{
	T_RTTI_CLASS;

public:
	SimpleBufferHeapDx11(ContextDx11* context, const D3D11_BUFFER_DESC& dbd);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool alloc(uint32_t bufferSize, uint32_t vertexStride, Chunk& outChunk) T_OVERRIDE T_FINAL;

	virtual void free(Chunk& chunk) T_OVERRIDE T_FINAL;

private:
	ContextDx11* m_context;
	D3D11_BUFFER_DESC m_dbd;
};

	}
}

#endif	// traktor_render_SimpleBufferHeapDx11_H
