/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_CubeTextureDx9_H
#define traktor_render_CubeTextureDx9_H

#include "Core/Misc/ComRef.h"
#include "Render/ICubeTexture.h"
#include "Render/Types.h"
#include "Render/Dx9/IResourceDx9.h"
#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

struct CubeTextureCreateDesc;
class ResourceManagerDx9;

/*!
 * \ingroup DX9 Xbox360
 */
class CubeTextureDx9
:	public ICubeTexture
,	public IResourceDx9
{
	T_RTTI_CLASS;

public:
	CubeTextureDx9(ResourceManagerDx9* resourceManager);

	virtual ~CubeTextureDx9();

	bool create(IDirect3DDevice9* d3dDevice, const CubeTextureCreateDesc& desc);

	// \name ITexture
	// \{

	virtual void destroy();

	virtual ITexture* resolve();

	// \}

	// \name ICubeTexture
	// \{

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual bool lock(int side, int level, Lock& lock);

	virtual void unlock(int side, int level);

	// \}

	// \name IResourceDx9
	// \{

	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

	// \}

	IDirect3DBaseTexture9* getD3DBaseTexture() const { return m_d3dCubeTexture; }
	
private:
	Ref< ResourceManagerDx9 > m_resourceManager;
	ComRef< IDirect3DCubeTexture9 > m_d3dCubeTexture;
	TextureFormat m_format;
	int m_side;
	void* m_lock;
};
		
	}
}

#endif	// traktor_render_CubeTextureDx9_H
