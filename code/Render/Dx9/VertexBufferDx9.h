#ifndef traktor_render_VertexBufferDx9_H
#define traktor_render_VertexBufferDx9_H

#include "Render/VertexBuffer.h"
#include "Render/Dx9/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX9_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX9 Xbox360
 */
class T_DLLCLASS VertexBufferDx9 : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	VertexBufferDx9(uint32_t bufferSize);

	virtual bool activate(IDirect3DDevice9* d3dDevice) = 0;
};
	
	}
}

#endif	// traktor_render_VertexBufferDx9_H
