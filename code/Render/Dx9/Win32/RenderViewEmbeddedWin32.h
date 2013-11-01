#ifndef traktor_render_RenderViewEmbeddedWin32_H
#define traktor_render_RenderViewEmbeddedWin32_H

#include "Render/Dx9/Win32/RenderViewWin32.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX9
 */
class RenderViewEmbeddedWin32
:	public RenderViewWin32
{
	T_RTTI_CLASS;

public:
	RenderViewEmbeddedWin32(
		RenderSystemWin32* renderSystem,
		ClearTarget* clearTarget,
		ParameterCache* parameterCache,
		IDirect3DDevice9* d3dDevice,
		const D3DPRESENT_PARAMETERS& d3dPresent,
		D3DFORMAT d3dDepthStencilFormat
	);

	virtual ~RenderViewEmbeddedWin32();

	virtual bool nextEvent(RenderEvent& outEvent);

	virtual bool reset(const RenderViewDefaultDesc& desc);

	virtual bool reset(int32_t width, int32_t height);

	virtual int getWidth() const;

	virtual int getHeight() const;

	virtual bool isActive() const;

	virtual bool isFullScreen() const;

	virtual void showCursor();

	virtual void hideCursor();

	virtual bool isCursorVisible() const;

	virtual SystemWindow getSystemWindow();

	// \name Swap-chain management
	// \{

	HRESULT lostDevice();

	HRESULT resetDevice();

	// \}

private:
	D3DPRESENT_PARAMETERS m_d3dPresent;
	D3DFORMAT m_d3dDepthStencilFormat;
};

	}
}

#endif	// traktor_render_RenderViewEmbeddedWin32_H
