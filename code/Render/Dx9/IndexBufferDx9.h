/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IndexBufferDx9_H
#define traktor_render_IndexBufferDx9_H

#include "Core/Misc/ComRef.h"
#include "Render/IndexBuffer.h"
#include "Render/Dx9/IResourceDx9.h"

namespace traktor
{
	namespace render
	{

class ResourceManagerDx9;

/*!
 * \ingroup DX9 Xbox360
 */
class IndexBufferDx9
:	public IndexBuffer
,	public IResourceDx9
{
	T_RTTI_CLASS;

public:
	IndexBufferDx9(ResourceManagerDx9* resourceManager, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferDx9();

	bool create(IDirect3DDevice9* d3dDevice, bool dynamic);

	static void activate(IDirect3DDevice9* d3dDevice, IndexBufferDx9* indexBuffer);

	// \name IndexBuffer
	// \{

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	// \}

	// \name IResourceDx9
	// \{

	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

	// \}

private:
	static IndexBufferDx9* ms_activeIndexBuffer;
	Ref< ResourceManagerDx9 > m_resourceManager;
	ComRef< IDirect3DIndexBuffer9 > m_d3dIndexBuffer;
	D3DFORMAT m_d3dFormat;
	bool m_dynamic;
};
	
	}
}

#endif	// traktor_render_IndexBufferDx9_H
