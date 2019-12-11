#pragma once

#include "Render/Dx11/IBufferHeapDx11.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*! Manages DX11 buffer memory.
 * \ingroup DX11
 */
class SimpleBufferHeapDx11 : public IBufferHeapDx11
{
	T_RTTI_CLASS;

public:
	SimpleBufferHeapDx11(ContextDx11* context, const D3D11_BUFFER_DESC& dbd);

	virtual void destroy() override final;

	virtual bool alloc(uint32_t bufferSize, uint32_t vertexStride, Chunk& outChunk) override final;

	virtual void free(Chunk& chunk) override final;

private:
	ContextDx11* m_context;
	D3D11_BUFFER_DESC m_dbd;
};

	}
}

