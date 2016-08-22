#ifndef traktor_render_SimpleTextureDx9_H
#define traktor_render_SimpleTextureDx9_H

#include "Core/Misc/ComRef.h"
#include "Render/ISimpleTexture.h"
#include "Render/Dx9/IResourceDx9.h"

namespace traktor
{
	namespace render
	{

class ResourceManagerDx9;
struct SimpleTextureCreateDesc;

/*!
 * \ingroup DX9 Xbox360
 */
class SimpleTextureDx9
:	public ISimpleTexture
,	public IResourceDx9
{
	T_RTTI_CLASS;

public:
	SimpleTextureDx9(ResourceManagerDx9* resourceManager);

	virtual ~SimpleTextureDx9();

	bool create(IDirect3DDevice9* d3dDevice, const SimpleTextureCreateDesc& desc);

	// \name ITexture
	// \{

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	// \}

	// \name ISimpleTexture
	// \{

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

	// \}

	// \name IResourceDx9
	// \{

	virtual HRESULT lostDevice() T_OVERRIDE T_FINAL;

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice) T_OVERRIDE T_FINAL;

	// \}

	IDirect3DBaseTexture9* getD3DBaseTexture() const { return m_d3dTexture; }

private:
	Ref< ResourceManagerDx9 > m_resourceManager;
	ComRef< IDirect3DTexture9 > m_d3dTexture;
	TextureFormat m_format;
	int m_width;
	int m_height;
	void* m_lock;
};
		
	}
}

#endif	// traktor_render_SimpleTextureDx9_H
