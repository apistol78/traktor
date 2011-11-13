#ifndef traktor_render_VertexBufferDx11_H
#define traktor_render_VertexBufferDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/VertexBuffer.h"
#include "Render/Dx11/Platform.h"

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

/*!
 * \ingroup DX11
 */
class T_DLLCLASS VertexBufferDx11 : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	virtual void prepare() = 0;

	ID3D11Buffer* getD3D11Buffer() const { return m_d3dBuffer; }

	UINT getD3D11Stride() const { return m_d3dStride; }

	const std::vector< D3D11_INPUT_ELEMENT_DESC >& getD3D11InputElements() const { return m_d3dInputElements; }

protected:
	ComRef< ID3D11Buffer > m_d3dBuffer;
	UINT m_d3dStride;
	std::vector< D3D11_INPUT_ELEMENT_DESC > m_d3dInputElements;

	VertexBufferDx11(uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_VertexBufferDx11_H
