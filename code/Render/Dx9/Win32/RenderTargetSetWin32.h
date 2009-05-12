#ifndef traktor_render_RenderTargetSetWin32_H
#define traktor_render_RenderTargetSetWin32_H

#include "Render/RenderTargetSet.h"
#include "Render/Types.h"
#include "Render/Dx9/Unmanaged.h"
#include "Core/Heap/Ref.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX9_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderTargetWin32;
class ContextDx9;

/*!
 * \ingroup DX9
 */
class T_DLLCLASS RenderTargetSetWin32
:	public RenderTargetSet
,	public Unmanaged
{
	T_RTTI_CLASS(RenderTargetSetWin32)

public:
	RenderTargetSetWin32(UnmanagedListener* unmanagedListener, ContextDx9* context);

	virtual ~RenderTargetSetWin32();

	bool create(IDirect3DDevice9* d3dDevice, const RenderTargetSetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual Texture* getColorTexture(int index) const;

	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

	inline RenderTargetWin32* getRenderTarget(int renderTarget) const {
		return m_colorTextures[renderTarget];
	}

	inline IDirect3DSurface9* getD3DDepthStencilSurface() const {
		return m_d3dTargetDepthStencilSurface;
	}

private:
	Ref< ContextDx9 > m_context;
	RenderTargetSetCreateDesc m_desc;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	ComRef< IDirect3DTexture9 > m_d3dTargetDepthStencilTexture;
	ComRef< IDirect3DSurface9 > m_d3dTargetDepthStencilSurface;
	RefArray< RenderTargetWin32 > m_colorTextures;

	LRESULT internalCreate();
};

	}
}

#endif	// traktor_render_RenderTargetSetWin32_H
