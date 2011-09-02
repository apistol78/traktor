#ifndef traktor_render_RenderTargetSetWin32_H
#define traktor_render_RenderTargetSetWin32_H

#include "Core/RefArray.h"
#include "Core/Misc/ComRef.h"
#include "Render/RenderTargetSet.h"
#include "Render/Types.h"
#include "Render/Dx9/IResourceDx9.h"

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

class RenderTargetWin32;
class ResourceManagerDx9;

/*!
 * \ingroup DX9
 */
class T_DLLCLASS RenderTargetSetWin32
:	public RenderTargetSet
,	public IResourceDx9
{
	T_RTTI_CLASS;

public:
	RenderTargetSetWin32(ResourceManagerDx9* resourceManager);

	virtual ~RenderTargetSetWin32();

	bool create(IDirect3DDevice9* d3dDevice, const RenderTargetSetCreateDesc& desc);

	// \name RenderTargetSet
	// \{

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual Ref< ISimpleTexture > getColorTexture(int index) const;

	virtual void swap(int index1, int index2);

	virtual bool read(int index, void* buffer) const;

	// \}

	// \name IResourceDx9
	// \{

	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

	// \}

	RenderTargetWin32* getRenderTarget(int renderTarget) const { return m_colorTextures[renderTarget]; }

	IDirect3DSurface9* getD3DDepthStencilSurface() const { return m_d3dTargetDepthStencilSurface; }

	bool usingPrimaryDepthStencil() const { return m_desc.usingPrimaryDepthStencil; }

private:
	Ref< ResourceManagerDx9 > m_resourceManager;
	RenderTargetSetCreateDesc m_desc;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	ComRef< IDirect3DTexture9 > m_d3dTargetDepthStencilTexture;
	ComRef< IDirect3DSurface9 > m_d3dTargetDepthStencilSurface;
	RefArray< RenderTargetWin32 > m_colorTextures;

	HRESULT internalCreate();
};

	}
}

#endif	// traktor_render_RenderTargetSetWin32_H
