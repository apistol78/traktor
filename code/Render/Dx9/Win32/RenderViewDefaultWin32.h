#ifndef traktor_render_RenderViewDefaultWin32_H
#define traktor_render_RenderViewDefaultWin32_H

#include "Render/Dx9/Win32/RenderViewWin32.h"
#include "Render/Dx9/Win32/Window.h"

namespace traktor
{
	namespace render
	{

/*! \brief Default DX9 render view implementation.
 *
 * This uses the implicit swap chain owned by the
 * DX9 device thus only a single default view
 * can be created at any time.
 *
 * \ingroup DX9
 */
class RenderViewDefaultWin32
:	public RenderViewWin32
,	public IWindowListener
{
	T_RTTI_CLASS;

public:
	RenderViewDefaultWin32(
		RenderSystemWin32* renderSystem,
		ClearTarget* clearTarget,
		ParameterCache* parameterCache,
		IDirect3DDevice9* d3dDevice,
		IDirect3D9* d3d,
		D3DPRESENT_PARAMETERS& d3dPresent,
		Window* window
	);

	virtual ~RenderViewDefaultWin32();

	virtual void close() T_OVERRIDE T_FINAL;

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

	virtual SystemWindow getSystemWindow() T_OVERRIDE T_FINAL;

	// \name Swap-chain management
	// \{

	virtual HRESULT lostDevice() T_OVERRIDE T_FINAL;

	virtual HRESULT resetDevice() T_OVERRIDE T_FINAL;

	// \}

private:
	ComRef< IDirect3D9 > m_d3d;
	D3DPRESENT_PARAMETERS& m_d3dPresent;
	Ref< Window > m_window;
	D3DFORMAT m_d3dDepthStencilFormat;
	std::list< RenderEvent > m_eventQueue;

	// \name IWindowListener implementation.
	// \{

	virtual bool windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult);

	// \}
};

	}
}

#endif	// traktor_render_RenderViewDefaultWin32_H
