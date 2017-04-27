/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VolumeTextureDx9_H
#define traktor_render_VolumeTextureDx9_H

#include "Core/Misc/ComRef.h"
#include "Render/IVolumeTexture.h"
#include "Render/Dx9/IResourceDx9.h"

namespace traktor
{
	namespace render
	{

class ResourceManagerDx9;
struct VolumeTextureCreateDesc;
		
/*!
 * \ingroup DX9 Xbox360
 */
class VolumeTextureDx9
:	public IVolumeTexture
,	public IResourceDx9
{
	T_RTTI_CLASS;
	
public:
	VolumeTextureDx9(ResourceManagerDx9* resourceManager);

	virtual ~VolumeTextureDx9();

	bool create(IDirect3DDevice9* d3dDevice, const VolumeTextureCreateDesc& desc);

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

	// \}

	// \name IResourceDx9
	// \{

	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

	// \}

	IDirect3DBaseTexture9* getD3DBaseTexture() const { return m_d3dVolumeTexture; }
	
private:
	Ref< ResourceManagerDx9 > m_resourceManager;
	ComRef< IDirect3DVolumeTexture9 > m_d3dVolumeTexture;
	TextureFormat m_format;
	int m_width;
	int m_height;
	int m_depth;
};
		
	}
}

#endif	// traktor_render_VolumeTextureDx9_H
