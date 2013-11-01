#include "Core/Log/Log.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Dx9/Win32/RenderSystemWin32.h"
#include "Render/Dx9/Win32/RenderViewDefaultWin32.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct RenderEventTypePred
{
	RenderEventType m_type;

	RenderEventTypePred(RenderEventType type)
	:	m_type(type)
	{
	}

	bool operator () (const RenderEvent& evt) const
	{
		return evt.type == m_type;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewDefaultWin32", RenderViewDefaultWin32, RenderViewWin32)

RenderViewDefaultWin32::RenderViewDefaultWin32(
	RenderSystemWin32* renderSystem,
	ClearTarget* clearTarget,
	ParameterCache* parameterCache,
	IDirect3DDevice9* d3dDevice,
	IDirect3D9* d3d,
	D3DPRESENT_PARAMETERS& d3dPresent,
	Window* window
)
:	RenderViewWin32(renderSystem, clearTarget, parameterCache, d3dDevice)
,	m_d3d(d3d)
,	m_d3dPresent(d3dPresent)
,	m_window(window)
,	m_d3dDepthStencilFormat(D3DFMT_UNKNOWN)
{
	m_window->addListener(this);
}

RenderViewDefaultWin32::~RenderViewDefaultWin32()
{
	close();
}

void RenderViewDefaultWin32::close()
{
	m_window->removeListener(this);
	m_window->hide();

	RenderViewWin32::close();
}

bool RenderViewDefaultWin32::nextEvent(RenderEvent& outEvent)
{
	MSG msg;

	for (;;)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (m_renderSystem->tryRecoverDevice())
			break;
		else
		{
			// Device is lost; spin in this method as long as device is lost
			// since we don't want application to progress "invisibly".
			Sleep(100);
		}
	}

	if (!m_eventQueue.empty())
	{
		outEvent = m_eventQueue.front();
		m_eventQueue.pop_front();
		return true;
	}
	else
		return false;
}

bool RenderViewDefaultWin32::reset(const RenderViewDefaultDesc& desc)
{
	D3DMULTISAMPLE_TYPE d3dMultiSample;
	HRESULT hr;

	T_DEBUG(L"Reset implicit DX9 render view, " << desc.displayMode.width << L"*" << desc.displayMode.height << L", " << (desc.fullscreen ? L"fullscreen" : L"window"));

	// Determine safe formats.
	m_d3dDepthStencilFormat = determineDepthStencilFormat(m_d3d, desc.depthBits, desc.stencilBits, D3DFMT_X8R8G8B8);
	if (m_d3dDepthStencilFormat == D3DFMT_UNKNOWN)
		return false;

	d3dMultiSample = determineMultiSampleType(m_d3d, D3DFMT_X8R8G8B8, m_d3dDepthStencilFormat, desc.multiSample);
	if (d3dMultiSample == D3DMULTISAMPLE_FORCE_DWORD)
		return false;

	// Setup presentation parameters.
	std::memset(&m_d3dPresent, 0, sizeof(m_d3dPresent));
	m_d3dPresent.BackBufferFormat = D3DFMT_X8R8G8B8;
	m_d3dPresent.BackBufferCount = 1;
	m_d3dPresent.BackBufferWidth = desc.displayMode.width;
	m_d3dPresent.BackBufferHeight = desc.displayMode.height;
	m_d3dPresent.MultiSampleType = d3dMultiSample;
	m_d3dPresent.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dPresent.hDeviceWindow = *m_window;
	m_d3dPresent.Windowed = desc.fullscreen ? FALSE : TRUE;
	m_d3dPresent.EnableAutoDepthStencil = FALSE;
	m_d3dPresent.PresentationInterval = desc.waitVBlank ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;
	m_d3dPresent.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	// Modify render window; temporarily remove ourself as listener as we don't want resize events during
	// style change.
	m_window->removeListener(this);

	if (!m_d3dPresent.Windowed)
		m_window->setFullScreenStyle(desc.displayMode.width, desc.displayMode.height);

	hr = m_renderSystem->resetDevice();

	if (m_d3dPresent.Windowed)
	{
		m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - DirectX 9.0c Renderer");
		m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);
	}

	m_window->addListener(this);

	return SUCCEEDED(hr);
}

bool RenderViewDefaultWin32::reset(int32_t width, int32_t height)
{
	HRESULT hr;

	if (!width || !height)
		return false;
	if (m_d3dPresent.BackBufferWidth == width && m_d3dPresent.BackBufferHeight == height)
		return true;

	m_d3dPresent.BackBufferWidth = width;
	m_d3dPresent.BackBufferHeight = height;

	hr = m_renderSystem->resetDevice();
	return SUCCEEDED(hr);
}

int RenderViewDefaultWin32::getWidth() const
{
	return m_d3dPresent.BackBufferWidth;
}

int RenderViewDefaultWin32::getHeight() const
{
	return m_d3dPresent.BackBufferHeight;
}

bool RenderViewDefaultWin32::isActive() const
{
	if (m_d3dDevice)
		return GetForegroundWindow() == m_d3dPresent.hDeviceWindow;
	else
		return false;
}

bool RenderViewDefaultWin32::isFullScreen() const
{
	return !m_d3dPresent.Windowed;
}

void RenderViewDefaultWin32::showCursor()
{
}

void RenderViewDefaultWin32::hideCursor()
{
}

bool RenderViewDefaultWin32::isCursorVisible() const
{
	return true;
}

SystemWindow RenderViewDefaultWin32::getSystemWindow()
{
	SystemWindow sw;
	sw.hWnd = m_d3dPresent.hDeviceWindow;
	return sw;
}

HRESULT RenderViewDefaultWin32::lostDevice()
{
	m_d3dSwapChain.release();
	m_d3dBackBuffer.release();
	m_d3dDepthStencilSurface.release();

	for (uint32_t i = 0; i < sizeof_array(m_d3dSyncQueries); ++i)
		m_d3dSyncQueries[i].release();

	m_renderStateStack.clear();
	return S_OK;
}

HRESULT RenderViewDefaultWin32::resetDevice()
{
	HRESULT hr;

	hr = m_d3dDevice->GetSwapChain(0, &m_d3dSwapChain.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to get implicit swap chain; hr = " << hr << Endl;
		return hr;
	}

	hr = m_d3dSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_d3dBackBuffer.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to get back buffer; hr = " << hr << Endl;
		return hr;
	}

	hr = m_d3dDevice->CreateDepthStencilSurface(
		m_d3dPresent.BackBufferWidth,
		m_d3dPresent.BackBufferHeight,
		m_d3dDepthStencilFormat,
		m_d3dPresent.MultiSampleType,
		0,
		TRUE,
		&m_d3dDepthStencilSurface.getAssign(),
		NULL
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to get depth/stencil surface; hr = " << hr << Endl;
		return hr;
	}

	for (uint32_t i = 0; i < sizeof_array(m_d3dSyncQueries); ++i)
	{
		hr = m_d3dDevice->CreateQuery(
			D3DQUERYTYPE_EVENT,
			&m_d3dSyncQueries[i].getAssign()
		);
		if (FAILED(hr))
		{
			log::warning << L"Unable to create synchronization query; hr = " << hr << Endl;
			m_d3dSyncQueries[i].release();
		}
	}

	m_d3dViewport.X = 0;
	m_d3dViewport.Y = 0;
	m_d3dViewport.Width = m_d3dPresent.BackBufferWidth;
	m_d3dViewport.Height = m_d3dPresent.BackBufferHeight;
	m_d3dViewport.MinZ = 0.0f;
	m_d3dViewport.MaxZ = 1.0f;

	return S_OK;
}

bool RenderViewDefaultWin32::windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
{
	if (message == WM_CLOSE)
	{
		RenderEvent evt;
		evt.type = ReClose;
		m_eventQueue.push_back(evt);
	}
	else if (message == WM_SIZE)
	{
		// Remove all pending resize events.
		m_eventQueue.remove_if(RenderEventTypePred(ReResize));

		// Push new resize event if not matching current size.
		int32_t width = LOWORD(lParam);
		int32_t height = HIWORD(lParam);
		if (width != m_d3dPresent.BackBufferWidth || height != m_d3dPresent.BackBufferHeight)
		{
			RenderEvent evt;
			evt.type = ReResize;
			evt.resize.width = width;
			evt.resize.height = height;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_SIZING)
	{
		RECT* rcWindowSize = (RECT*)lParam;

		int32_t width = rcWindowSize->right - rcWindowSize->left;
		int32_t height = rcWindowSize->bottom - rcWindowSize->top;

		if (width < 320)
			width = 320;
		if (height < 200)
			height = 200;

		if (wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_BOTTOMRIGHT)
			rcWindowSize->right = rcWindowSize->left + width;
		if (wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
			rcWindowSize->left = rcWindowSize->right - width;

		if (wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT)
			rcWindowSize->bottom = rcWindowSize->top + height;
		if (wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)
			rcWindowSize->top = rcWindowSize->bottom - height;

		outResult = TRUE;
	}
	else if (message == WM_SYSKEYDOWN)
	{
		if (wParam == VK_RETURN && (lParam & (1 << 29)) != 0)
		{
			RenderEvent evt;
			evt.type = ReToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_KEYDOWN)
	{
		if (wParam == VK_RETURN && (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
		{
			RenderEvent evt;
			evt.type = ReToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_SETCURSOR)
	{
		if (m_d3dPresent.Windowed)
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		else
			SetCursor(NULL);
	}
	else
		return false;

	return true;
}

	}
}