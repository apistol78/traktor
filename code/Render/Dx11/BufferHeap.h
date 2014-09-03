#ifndef traktor_render_BufferHeap_H
#define traktor_render_BufferHeap_H

#include <list>
#include <map>
#include "Core/Object.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*! \brief Manages DX11 buffer memory.
 * \ingroup DX11
 *
 * This virtualizes the DX11 buffers from VertexBuffer instances; thus permit
 * larger chunks of memory being used and shared between multiple instances.
 *
 * This reduces the number of state changes required by the driver thus
 * improves performance.
 */
class BufferHeap : public Object
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

	BufferHeap(ContextDx11* context, const D3D11_BUFFER_DESC& dbd);

	void destroy();

	bool alloc(uint32_t bufferSize, uint32_t vertexStride, Chunk& outChunk);

	void free(const Chunk& chunk);

private:
	struct FreeList
	{
		uint32_t offset;
		uint32_t size;

		bool operator < (const FreeList& rh) const;
	};

	struct Chain
	{
		ComRef< ID3D11Buffer > d3dBuffer;
		std::vector< FreeList > freeList;
	};

	ContextDx11* m_context;
	D3D11_BUFFER_DESC m_dbd;
	std::map< uint32_t, std::list< Chain > > m_chains;
};

	}
}

#endif	// traktor_render_BufferHeap_H
