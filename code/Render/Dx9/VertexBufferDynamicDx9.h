/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexBufferDynamicDx9_H
#define traktor_render_VertexBufferDynamicDx9_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/ComRef.h"
#include "Render/VertexElement.h"
#include "Render/Dx9/IResourceDx9.h"
#include "Render/Dx9/VertexBufferDx9.h"

namespace traktor
{
	namespace render
	{

class RenderViewDx9;
class ResourceManagerDx9;
class VertexDeclCache;

/*!
 * \ingroup DX9 Xbox360
 */
class VertexBufferDynamicDx9
:	public VertexBufferDx9
,	public IResourceDx9
{
	T_RTTI_CLASS;

public:
	VertexBufferDynamicDx9(ResourceManagerDx9* resourceManager, uint32_t bufferSize, VertexDeclCache* vertexDeclCache);

	virtual ~VertexBufferDynamicDx9();

	bool create(IDirect3DDevice9* d3dDevice, const AlignedVector< VertexElement >& vertexElements);

	virtual bool activate(IDirect3DDevice9* d3dDevice);

	// \name VertexBuffer
	// \{

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);
	
	virtual void unlock();

	// \}

	// \name IResourceDx9
	// \{

	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

	// \}

private:
	Ref< ResourceManagerDx9 > m_resourceManager;
	Ref< VertexDeclCache > m_vertexDeclCache;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	ComRef< IDirect3DVertexDeclaration9 > m_d3dVertexDeclaration;
	DWORD m_d3dVertexStride;
	ComRef< IDirect3DVertexBuffer9 > m_d3dVertexBuffer;
	AlignedVector< uint8_t > m_buffer;
	uint32_t m_dirtyRegion[2];
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_VertexBufferDynamicDx9_H
