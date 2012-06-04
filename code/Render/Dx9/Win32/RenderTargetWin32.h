#ifndef traktor_render_RenderTargetWin32_H
#define traktor_render_RenderTargetWin32_H

#include "Core/Math/Color4f.h"
#include "Core/Misc/ComRef.h"
#include "Render/ISimpleTexture.h"
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

struct RenderTargetSetCreateDesc;
struct RenderTargetCreateDesc;

class T_DLLCLASS RenderTargetWin32 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	RenderTargetWin32();

	bool create(
		IDirect3DDevice9* d3dDevice,
		const RenderTargetSetCreateDesc& setDesc,
		const RenderTargetCreateDesc& targetDesc,
		D3DFORMAT d3dFormat
	);

	void release();

	bool resolve(IDirect3DDevice9* d3dDevice);

	// \name ITexture
	// \{

	virtual void destroy();

	virtual ITexture* resolve();

	// \}

	// \name ISimpleTexture
	// \{

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	// \}

	IDirect3DBaseTexture9* getD3DBaseTexture() const { return m_d3dTargetTexture; }

	IDirect3DSurface9* getD3DColorSurface() const { return m_d3dTargetSurface; }

	D3DFORMAT getD3DFormat() const { return m_d3dFormat; }

private:
	int m_width;
	int m_height;
	ComRef< IDirect3DTexture9 > m_d3dTargetTexture;
	ComRef< IDirect3DSurface9 > m_d3dTargetSurface;
	ComRef< IDirect3DSurface9 > m_d3dResolveTargetSurface;
	D3DFORMAT m_d3dFormat;
};

	}
}

#endif	// traktor_render_RenderTargetWin32_H
