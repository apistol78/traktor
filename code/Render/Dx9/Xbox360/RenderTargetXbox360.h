#ifndef traktor_render_RenderTargetXbox360_H
#define traktor_render_RenderTargetXbox360_H

#include "Render/ITexture.h"
#include "Render/Dx9/TextureBaseDx9.h"
#include "Render/Dx9/Xbox360/RenderTargetPool.h"
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

class RenderSystemXbox360;

/*!
 * \ingroup Xbox360
 */
class T_DLLCLASS RenderTargetXbox360
:	public ITexture
,	public TextureBaseDx9
{
	T_RTTI_CLASS;

public:
	RenderTargetXbox360(RenderSystemXbox360* renderSystem);

	virtual ~RenderTargetXbox360();

	bool create(
		IDirect3DDevice9* d3dDevice,
		RenderTargetPool* pool,
		int width,
		int height,
		D3DFORMAT d3dFormat,
		D3DMULTISAMPLE_TYPE d3dMultisample,
		bool attachDepthStencil,
		bool doubleBuffered
	);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual IDirect3DBaseTexture9* getD3DBaseTexture() const;

	bool begin(IDirect3DDevice9* d3dDevice);

	void end(IDirect3DDevice9* d3dDevice);

	/*!
	 * \brief Cycle resolve targets.
	 * Used only for double buffered targets.
	 */
	void cycleTargets();

private:
	int m_width;
	int m_height;
	D3DFORMAT m_d3dFormat;
	D3DMULTISAMPLE_TYPE m_d3dMultisample;

	int m_acquireWidth;
	int m_acquireHeight;

	int m_tileCount;
	D3DRECT m_tileRects[4];

	Ref< RenderTargetPool > m_pool;
	RenderTargetPool::Target* m_acquiredTarget;

	ComRef< IDirect3DTexture9 > m_d3dTargetTexture;
	ComRef< IDirect3DTexture9 > m_d3dSecondaryTargetTexture;
	ComRef< IDirect3DSurface9 > m_d3dTargetDepthStencilSurface;
};

	}
}

#endif	// traktor_render_RenderTargetXbox360_H
