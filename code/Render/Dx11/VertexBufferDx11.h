#ifndef traktor_render_VertexBufferDx11_H
#define traktor_render_VertexBufferDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/VertexBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*!
 * \ingroup DX11
 */
class T_DLLCLASS VertexBufferDx11 : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	VertexBufferDx11(
		ContextDx11* context,
		uint32_t bufferSize,
		ID3D11Buffer* d3dBuffer,
		UINT d3dStride,
		const std::vector< D3D11_INPUT_ELEMENT_DESC >& d3dInputElements
	);

	virtual ~VertexBufferDx11();

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);
	
	virtual void unlock();

	ID3D11Buffer* getD3D11Buffer() const;

	UINT getD3D11Stride() const;

	const std::vector< D3D11_INPUT_ELEMENT_DESC >& getD3D11InputElements() const;

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Buffer > m_d3dBuffer;
	UINT m_d3dStride;
	std::vector< D3D11_INPUT_ELEMENT_DESC > m_d3dInputElements;
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_VertexBufferDx11_H
