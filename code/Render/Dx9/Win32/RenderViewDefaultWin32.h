#ifndef traktor_render_RenderViewDefaultWin32_H
#define traktor_render_RenderViewDefaultWin32_H

#include "Render/Dx9/Win32/RenderViewWin32.h"
#include "Render/Dx9/Win32/Window.h"

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

/*!
 * \ingroup DX9
 */
class T_DLLCLASS RenderViewDefaultWin32
:	public RenderViewWin32
,	public IWindowListener
{
	T_RTTI_CLASS;

public:
	RenderViewDefaultWin32(
		RenderSystemWin32* renderSystem,
		ParameterCache* parameterCache,
		IDirect3DDevice9* d3dDevice,
		IDirect3D9* d3d,
		Window* window
	);

	virtual ~RenderViewDefaultWin32();

	virtual void close();

	virtual bool nextEvent(RenderEvent& outEvent);

	virtual bool reset(const RenderViewDefaultDesc& desc);

	virtual void resize(int32_t width, int32_t height);

	virtual int getWidth() const;

	virtual int getHeight() const;

	virtual bool isActive() const;

	virtual bool isFullScreen() const;

	// \name Swap-chain management
	// \{

	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice();

	// \}

private:
	ComRef< IDirect3D9 > m_d3d;
	Window* m_window;
	D3DPRESENT_PARAMETERS m_d3dPresent;
	D3DFORMAT m_d3dDepthStencilFormat;
	std::list< RenderEvent > m_eventQueue;

	// \name IWindowListener implementation.
	// \{

	virtual void windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam);

	// \}
};

	}
}

#endif	// traktor_render_RenderViewDefaultWin32_H
