/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexBufferStaticDx9_H
#define traktor_render_VertexBufferStaticDx9_H

#include "Core/Misc/ComRef.h"
#include "Render/VertexElement.h"
#include "Render/Dx9/VertexBufferDx9.h"

namespace traktor
{
	namespace render
	{

class RenderViewDx9;
class VertexDeclCache;

/*!
 * \ingroup DX9 Xbox360
 */
class VertexBufferStaticDx9 : public VertexBufferDx9
{
	T_RTTI_CLASS;

public:
	VertexBufferStaticDx9(uint32_t bufferSize, VertexDeclCache* vertexDeclCache);

	virtual ~VertexBufferStaticDx9();

	bool create(IDirect3DDevice9* d3dDevice, const std::vector< VertexElement >& vertexElements);

	bool activate(IDirect3DDevice9* d3dDevice);

	// \name VertexBuffer
	// \{

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);
	
	virtual void unlock();

	// \}

private:
	Ref< VertexDeclCache > m_vertexDeclCache;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	ComRef< IDirect3DVertexDeclaration9 > m_d3dVertexDeclaration;
	DWORD m_d3dVertexStride;
	ComRef< IDirect3DVertexBuffer9 > m_d3dVertexBuffer;
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_VertexBufferStaticDx9_H
