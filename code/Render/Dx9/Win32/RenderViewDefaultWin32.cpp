#include "Core/Log/Log.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Dx9/Win32/RenderSystemWin32.h"
#include "Render/Dx9/Win32/RenderViewDefaultWin32.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewDefaultWin32", RenderViewDefaultWin32, RenderViewWin32)

RenderViewDefaultWin32::RenderViewDefaultWin32(
	RenderSystemWin32* renderSystem,
	ParameterCache* parameterCache,
	IDirect3DDevice9* d3dDevice,
	IDirect3D9* d3d,
	Window* window
)
:	RenderViewWin32(renderSystem, parameterCache, d3dDevice)
,	m_d3d(d3d)
,	m_window(window)
{
	std::memset(&m_d3dPresent, 0, sizeof(m_d3dPresent));
	m_window->addListener(this);
}

RenderViewDefaultWin32::~RenderViewDefaultWin32()
{
	close();
}

void RenderViewDefaultWin32::close()
{
	m_window->removeListener(this);
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
			Sleep(100);
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
	D3DFORMAT d3dDepthStencilFormat;
	D3DMULTISAMPLE_TYPE d3dMultiSample;
	HRESULT hr;

	// Determine safe formats.
	d3dDepthStencilFormat = determineDepthStencilFormat(m_d3d, desc.depthBits, desc.stencilBits, D3DFMT_X8R8G8B8);
	if (d3dDepthStencilFormat == D3DFMT_UNKNOWN)
		return false;

	d3dMultiSample = determineMultiSampleType(m_d3d, D3DFMT_X8R8G8B8, d3dDepthStencilFormat, desc.multiSample);
	if (d3dMultiSample == D3DMULTISAMPLE_FORCE_DWORD)
		return false;

	// Modify render window.
	if (m_d3dPresent.Windowed)
		m_window->setWindowedStyle();
	else
		m_window->setFullScreenStyle();

	m_window->setClientSize(m_d3dPresent.BackBufferWidth, m_d3dPresent.BackBufferHeight);
	m_window->show();

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

	hr = resetDevice();
	return SUCCEEDED(hr);
}

void RenderViewDefaultWin32::resize(int32_t width, int32_t height)
{
	T_ASSERT (m_renderStateStack.empty());

	if (!width || !height)
		return;
	if (m_d3dPresent.BackBufferWidth == width && m_d3dPresent.BackBufferHeight == height)
		return;

	m_d3dPresent.BackBufferWidth = width;
	m_d3dPresent.BackBufferHeight = height;

	resetDevice();
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

HRESULT RenderViewDefaultWin32::lostDevice()
{
	m_d3dDevice.release();
	m_d3dSwapChain.release();
	m_d3dBackBuffer.release();
	m_d3dDepthStencilSurface.release();

	for (uint32_t i = 0; i < sizeof_array(m_d3dSyncQueries); ++i)
		m_d3dSyncQueries[i].release();

	m_renderStateStack.clear();
	m_currentVertexBuffer = 0;
	m_currentIndexBuffer = 0;
	m_currentProgram = 0;

	return S_OK;
}

HRESULT RenderViewDefaultWin32::resetDevice()
{
	HRESULT hr;

	hr = m_d3dDevice->CreateAdditionalSwapChain(
		&m_d3dPresent,
		&m_d3dSwapChain.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create additional swap chain; hr = " << hr << Endl;
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

	return S_OK;
}

void RenderViewDefaultWin32::windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CLOSE)
	{
		RenderEvent evt;
		evt.type = ReClose;
		m_eventQueue.push_back(evt);
	}
	else if (message == WM_SIZE)
	{
		RenderEvent evt;
		evt.type = ReResize;
		evt.resize.width = LOWORD(lParam);
		evt.resize.height = HIWORD(lParam);
		m_eventQueue.push_back(evt);
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
}

	}
}