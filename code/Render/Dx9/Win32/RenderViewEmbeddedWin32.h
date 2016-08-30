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

	virtual bool nextEvent(RenderEvent& outEvent) T_OVERRIDE T_FINAL;

	virtual bool reset(const RenderViewDefaultDesc& desc) T_OVERRIDE T_FINAL;

	virtual bool reset(int32_t width, int32_t height) T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;

	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool isActive() const T_OVERRIDE T_FINAL;

	virtual bool isMinimized() const T_OVERRIDE T_FINAL;

	virtual bool isFullScreen() const T_OVERRIDE T_FINAL;

	virtual void showCursor() T_OVERRIDE T_FINAL;

	virtual void hideCursor() T_OVERRIDE T_FINAL;

	virtual bool isCursorVisible() const T_OVERRIDE T_FINAL;

	virtual SystemWindow getSystemWindow();

	// \name Swap-chain management
	// \{

	virtual HRESULT lostDevice() T_OVERRIDE T_FINAL;

	virtual HRESULT resetDevice() T_OVERRIDE T_FINAL;

	// \}

private:
	D3DPRESENT_PARAMETERS m_d3dPresent;
	D3DFORMAT m_d3dDepthStencilFormat;
};

	}
}

#endif	// traktor_render_RenderViewEmbeddedWin32_H
