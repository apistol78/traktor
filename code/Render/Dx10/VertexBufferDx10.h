#ifndef traktor_render_VertexBufferDx10_H
#define traktor_render_VertexBufferDx10_H

#include "Render/VertexBuffer.h"
#include "Core/Misc/ComRef.h"

namespace traktor
{
	namespace render
	{

class ContextDx10;

/*!
 * \ingroup DX10
 */
class VertexBufferDx10 : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	VertexBufferDx10(
		ContextDx10* context,
		uint32_t bufferSize,
		ID3D10Buffer* d3dBuffer,
		UINT d3dStride,
		const std::vector< D3D10_INPUT_ELEMENT_DESC >& d3dInputElements
	);

	virtual ~VertexBufferDx10();

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);
	
	virtual void unlock();

	ID3D10Buffer* getD3D10Buffer() const;

	UINT getD3D10Stride() const;

	const std::vector< D3D10_INPUT_ELEMENT_DESC >& getD3D10InputElements() const;

private:
	Ref< ContextDx10 > m_context;
	ComRef< ID3D10Buffer > m_d3dBuffer;
	UINT m_d3dStride;
	std::vector< D3D10_INPUT_ELEMENT_DESC > m_d3dInputElements;
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_VertexBufferDx10_H
