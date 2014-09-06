#ifndef traktor_render_IBufferHeapDx11_H
#define traktor_render_IBufferHeapDx11_H

#include "Core/Object.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

/*! \brief Manages DX11 buffer memory.
 * \ingroup DX11
 */
class IBufferHeapDx11 : public Object
{
	T_RTTI_CLASS;

public:
	struct Chunk
	{
		ID3D11Buffer* d3dBuffer;
		uint32_t byteOffset;
		uint32_t vertexOffset;
		uint32_t size;
	};

	virtual void destroy() = 0;

	virtual bool alloc(uint32_t bufferSize, uint32_t vertexStride, Chunk& outChunk) = 0;

	virtual void free(const Chunk& chunk) = 0;
};

	}
}

#endif	// traktor_render_IBufferHeapDx11_H
