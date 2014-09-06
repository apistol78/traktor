#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/SimpleBufferHeapDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleBufferHeapDx11", SimpleBufferHeapDx11, Object)

SimpleBufferHeapDx11::SimpleBufferHeapDx11(ContextDx11* context, const D3D11_BUFFER_DESC& dbd)
:	m_context(context)
,	m_dbd(dbd)
{
}

void SimpleBufferHeapDx11::destroy()
{
}

bool SimpleBufferHeapDx11::alloc(uint32_t bufferSize, uint32_t vertexStride, Chunk& outChunk)
{
	D3D11_BUFFER_DESC dbd = m_dbd;
	dbd.ByteWidth = bufferSize;

	HRESULT hr = m_context->getD3DDevice()->CreateBuffer(&dbd, NULL, &outChunk.d3dBuffer);
	if (FAILED(hr))
		return false;

	outChunk.byteOffset = 0;
	outChunk.vertexOffset = 0;
	outChunk.size = bufferSize;

	return true;
}

void SimpleBufferHeapDx11::free(const Chunk& chunk)
{
}

	}
}
