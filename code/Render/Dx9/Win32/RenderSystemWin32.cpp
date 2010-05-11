#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Thread/Acquire.h"
#include "Render/VertexElement.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Dx9/CubeTextureDx9.h"
#include "Render/Dx9/IndexBufferDx9.h"
#include "Render/Dx9/ParameterCache.h"
#include "Render/Dx9/ProgramResourceDx9.h"
#include "Render/Dx9/ResourceManagerDx9.h"
#include "Render/Dx9/ShaderCache.h"
#include "Render/Dx9/SimpleTextureDx9.h"
#include "Render/Dx9/VertexDeclCache.h"
#include "Render/Dx9/VertexBufferDx9.h"
#include "Render/Dx9/VolumeTextureDx9.h"
#include "Render/Dx9/Win32/ProgramWin32.h"
#include "Render/Dx9/Win32/ProgramCompilerWin32.h"
#include "Render/Dx9/Win32/RenderSystemWin32.h"
#include "Render/Dx9/Win32/RenderTargetSetWin32.h"
#include "Render/Dx9/Win32/RenderViewWin32.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const TCHAR* c_className = _T("TraktorRenderSystem");

uint16_t colorBitsFromFormat(D3DFORMAT d3dFormat)
{
	switch (d3dFormat)
	{
	case D3DFMT_A2R10G10B10:
	case D3DFMT_A8R8G8B8:
		return 32;
	case D3DFMT_X8R8G8B8:
		return 24;
	case D3DFMT_A1R5G5B5:
		return 16;
	case D3DFMT_X1R5G5B5:
	case D3DFMT_R5G6B5:
		return 15;
	}
	return 0;
}

void setWindowStyle(HWND hWnd, int32_t clientWidth, int32_t clientHeight, bool fullScreen)
{
	if (fullScreen)
	{
		SetWindowLong(hWnd, GWL_STYLE, WS_POPUPWINDOW);
		SetWindowPos(hWnd, HWND_TOP, 0, 0, clientWidth, clientHeight, SWP_FRAMECHANGED | SWP_NOMOVE);
	}
	else
	{
		SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX);
		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, clientWidth, clientHeight, SWP_FRAMECHANGED | SWP_NOMOVE);
	}

	RECT rcWindow, rcClient;
	GetWindowRect(hWnd, &rcWindow);
	GetClientRect(hWnd, &rcClient);

	int32_t windowWidth = rcWindow.right - rcWindow.left;
	int32_t windowHeight = rcWindow.bottom - rcWindow.top;

	int32_t realClientWidth = rcClient.right - rcClient.left;
	int32_t realClientHeight = rcClient.bottom - rcClient.top;

	windowWidth = (windowWidth - realClientWidth) + clientWidth;
	windowHeight = (windowHeight - realClientHeight) + clientHeight;

	if (fullScreen)
		SetWindowPos(hWnd, NULL, 0, 0, windowWidth, windowHeight, SWP_NOZORDER | SWP_NOMOVE);
	else
		SetWindowPos(hWnd, NULL, 128, 128, windowWidth, windowHeight, SWP_NOZORDER);

	ShowWindow(hWnd, SW_SHOWNOACTIVATE);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemWin32", 0, RenderSystemWin32, IRenderSystem)

RenderSystemWin32::RenderSystemWin32()
:	m_vertexDeclCache(0)
,	m_hWnd(0)
,	m_mipBias(0.0f)
,	m_maxAnisotropy(1)
,	m_lostDevice(false)
{
}

bool RenderSystemWin32::create(const RenderSystemCreateDesc& desc)
{
	HRESULT hr;

	m_d3d.getAssign() = Direct3DCreate9(D3D_SDK_VERSION);
	T_ASSERT (m_d3d);

	if (FAILED(m_d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &m_d3dDefaultDisplayMode)))
		return false;

	log::debug << L"Adapter display mode " << m_d3dDefaultDisplayMode.Width << L"x" << m_d3dDefaultDisplayMode.Height << L" " << m_d3dDefaultDisplayMode.RefreshRate << L" Hz" << Endl;

	WNDCLASS wc;
	std::memset(&wc, 0, sizeof(wc));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(this);
	wc.lpfnWndProc = (WNDPROC)wndProc;
	wc.hInstance = static_cast<HINSTANCE>(GetModuleHandle(0));
	wc.hIcon = NULL;
	wc.hCursor = static_cast<HCURSOR>(LoadCursor(NULL, IDC_ARROW));
	wc.lpszClassName = c_className;
	RegisterClass(&wc);

	m_hWnd = CreateWindow(
		c_className,
		_T("Traktor 2.0 DirectX 9.0 Renderer"),
		WS_POPUPWINDOW,
		0,
		0,
		0,
		0,
		NULL,
		NULL,
		static_cast< HMODULE >(GetModuleHandle(NULL)),
		this
	);
	if (!m_hWnd)
		return false;

	UINT d3dAdapter = D3DADAPTER_DEFAULT;
	D3DDEVTYPE d3dDevType = D3DDEVTYPE_HAL;

	// Create "resource" device.
	DWORD dwBehaviour =
		D3DCREATE_HARDWARE_VERTEXPROCESSING |
		D3DCREATE_FPU_PRESERVE |
		D3DCREATE_PUREDEVICE |
		D3DCREATE_MULTITHREADED;

	std::memset(&m_d3dPresent, 0, sizeof(m_d3dPresent));
	m_d3dPresent.BackBufferFormat = D3DFMT_UNKNOWN;
	m_d3dPresent.BackBufferCount = 1;
	m_d3dPresent.BackBufferWidth = 1;
	m_d3dPresent.BackBufferHeight = 1;
	m_d3dPresent.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_d3dPresent.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dPresent.hDeviceWindow = m_hWnd;
	m_d3dPresent.Windowed = TRUE;
	m_d3dPresent.EnableAutoDepthStencil = FALSE;
	m_d3dPresent.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	m_d3dPresent.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	hr = m_d3d->CreateDevice(
		d3dAdapter,
		d3dDevType,
		m_d3dPresent.hDeviceWindow,
		dwBehaviour,
		&m_d3dPresent,
		&m_d3dDevice.getAssign()
	);
	if (FAILED(hr) || !m_d3dDevice)
	{
		log::error << L"CreateDevice failed; unable to create device" << Endl;
		return false;
	}

	D3DCAPS9 d3dDeviceCaps;
	std::memset(&d3dDeviceCaps, 0, sizeof(d3dDeviceCaps));
	hr = m_d3dDevice->GetDeviceCaps(&d3dDeviceCaps);
	if (SUCCEEDED(hr))
	{
		// Ensure device supports at least SM 3.0.
		if (d3dDeviceCaps.VertexShaderVersion < D3DVS_VERSION(3, 0))
		{
			log::error << L"Create device failed, need at least VS 3.0 (device VS " << uint32_t(D3DSHADER_VERSION_MAJOR(d3dDeviceCaps.VertexShaderVersion)) << L"." << uint32_t(D3DSHADER_VERSION_MINOR(d3dDeviceCaps.VertexShaderVersion)) << L")" << Endl;
			return false;
		}
		if (d3dDeviceCaps.PixelShaderVersion < D3DPS_VERSION(3, 0))
		{
			log::error << L"Create device failed, need at least PS 3.0 (device PS " << uint32_t(D3DSHADER_VERSION_MAJOR(d3dDeviceCaps.PixelShaderVersion)) << L"." << uint32_t(D3DSHADER_VERSION_MINOR(d3dDeviceCaps.PixelShaderVersion)) << L")" << Endl;
			return false;
		}

		log::debug << L"D3DCAPS9.MaxVertexShaderConst = " << uint32_t(d3dDeviceCaps.MaxVertexShaderConst) << Endl;
	}
	else
		log::warning << L"Unable to get device capabilities; may produce unexpected results" << Endl;

	// Investigate available texture memory.
	UINT availTextureMem = m_d3dDevice->GetAvailableTextureMem();
	log::debug << L"Estimated " << availTextureMem / (1024 * 1024) << L" MiB texture memory available" << Endl;

	m_resourceManager = new ResourceManagerDx9();
	m_shaderCache = new ShaderCache();
	m_parameterCache = new ParameterCache(m_d3dDevice);
	m_vertexDeclCache = new VertexDeclCache(m_d3dDevice);

	m_mipBias = desc.mipBias;
	m_maxAnisotropy = desc.maxAnisotropy;

	return true;
}

void RenderSystemWin32::destroy()
{
	T_ASSERT (m_renderViews.empty());

	if (m_resourceManager)
	{
		m_resourceManager->lostDevice();
		m_resourceManager = 0;
	}

	if (m_parameterCache)
	{
		m_parameterCache->lostDevice();
		m_parameterCache = 0;
	}

	if (m_vertexDeclCache)
	{
		m_vertexDeclCache->lostDevice();
		m_vertexDeclCache = 0;
	}

	if (m_shaderCache)
	{
		m_shaderCache->releaseAll();
		m_shaderCache = 0;
	}

	if (m_hWnd)
	{
		SetWindowLongPtr(m_hWnd, 0, 0);
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}

	m_d3dDevice.release();
	m_d3d.release();
}

uint32_t RenderSystemWin32::getDisplayModeCount() const
{
	return uint32_t(m_d3d->GetAdapterModeCount(D3DADAPTER_DEFAULT, m_d3dDefaultDisplayMode.Format));
}

DisplayMode RenderSystemWin32::getDisplayMode(uint32_t index) const
{
	D3DDISPLAYMODE d3ddm;
	m_d3d->EnumAdapterModes(D3DADAPTER_DEFAULT, m_d3dDefaultDisplayMode.Format, index, &d3ddm);

	DisplayMode dm;
	dm.width = d3ddm.Width;
	dm.height = d3ddm.Height;
	dm.refreshRate = d3ddm.RefreshRate;
	dm.colorBits = colorBitsFromFormat(d3ddm.Format);
	return dm;
}

DisplayMode RenderSystemWin32::getCurrentDisplayMode() const
{
	DisplayMode dm;
	dm.width = m_d3dDefaultDisplayMode.Width;
	dm.height = m_d3dDefaultDisplayMode.Height;
	dm.refreshRate = m_d3dDefaultDisplayMode.RefreshRate;
	dm.colorBits = colorBitsFromFormat(m_d3dDefaultDisplayMode.Format);
	return dm;
}

bool RenderSystemWin32::handleMessages()
{
	bool going = true;
	MSG msg;
	
	// Dispatch pending window messages.
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret <= 0 || msg.message == WM_QUIT)
			going = false;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Try to reset lost device.
	if (m_lostDevice)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);
		HRESULT hr;

		hr = m_d3dDevice->TestCooperativeLevel();

		if (hr == D3DERR_DEVICENOTRESET)
			hr = resetDevice();

		if (SUCCEEDED(hr))
			m_lostDevice = false;
	}

	return going;
}

Ref< IRenderView > RenderSystemWin32::createRenderView(const RenderViewDefaultDesc& desc)
{
	D3DFORMAT d3dDepthStencilFormat;
	D3DMULTISAMPLE_TYPE d3dMultiSample;
	HRESULT hr;

	T_ASSERT (m_hWnd);
	T_ASSERT (m_renderViews.empty());

	setWindowStyle(m_hWnd, desc.displayMode.width, desc.displayMode.height, desc.fullscreen);

	if (desc.stencilBits == 1)
		d3dDepthStencilFormat = D3DFMT_D15S1;
	else if (desc.stencilBits > 1)
		d3dDepthStencilFormat = D3DFMT_D24S8;
	else
		d3dDepthStencilFormat = (desc.depthBits > 16) ? D3DFMT_D24X8 : D3DFMT_D16;

	d3dMultiSample = D3DMULTISAMPLE_NONE;
	
	hr = m_d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, FALSE, c_d3dMultiSample[desc.multiSample], NULL);
	if (SUCCEEDED(hr))
	{
		hr = m_d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dDepthStencilFormat, FALSE, c_d3dMultiSample[desc.multiSample], NULL);
		if (SUCCEEDED(hr))
			d3dMultiSample = c_d3dMultiSample[desc.multiSample];
	}

	std::memset(&m_d3dPresent, 0, sizeof(m_d3dPresent));
	m_d3dPresent.BackBufferFormat = D3DFMT_X8R8G8B8;
	m_d3dPresent.BackBufferCount = 1;
	m_d3dPresent.BackBufferWidth = desc.displayMode.width;
	m_d3dPresent.BackBufferHeight = desc.displayMode.height;
	m_d3dPresent.MultiSampleType = d3dMultiSample;
	m_d3dPresent.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dPresent.hDeviceWindow = m_hWnd;
	m_d3dPresent.Windowed = desc.fullscreen ? FALSE : TRUE;
	m_d3dPresent.EnableAutoDepthStencil = FALSE;
	m_d3dPresent.PresentationInterval = desc.waitVBlank ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;
	m_d3dPresent.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	hr = m_d3dDevice->Reset(&m_d3dPresent);
	if (FAILED(hr))
		return 0;

	Ref< RenderViewWin32 > renderView = new RenderViewWin32(
		this,
		m_parameterCache,
		desc,
		m_d3dPresent,
		d3dDepthStencilFormat
	);

	hr = renderView->resetDevice(m_d3dDevice);
	if (FAILED(hr))
		return 0;

	return renderView;
}

Ref< IRenderView > RenderSystemWin32::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	D3DPRESENT_PARAMETERS d3dPresent;
	D3DFORMAT d3dDepthStencilFormat;
	D3DMULTISAMPLE_TYPE d3dMultiSample;
	HRESULT hr;
	RECT rcWindow;

	GetClientRect((HWND)desc.nativeWindowHandle, &rcWindow);
	if (rcWindow.left >= rcWindow.right)
		rcWindow.right = rcWindow.left + 10;
	if (rcWindow.top >= rcWindow.bottom)
		rcWindow.bottom = rcWindow.top + 10;

	if (desc.stencilBits == 1)
		d3dDepthStencilFormat = D3DFMT_D15S1;
	else if (desc.stencilBits > 1)
		d3dDepthStencilFormat = D3DFMT_D24S8;
	else
		d3dDepthStencilFormat = (desc.depthBits > 16) ? D3DFMT_D24X8 : D3DFMT_D16;

	d3dMultiSample = D3DMULTISAMPLE_NONE;

	hr = m_d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, TRUE, c_d3dMultiSample[desc.multiSample], NULL);
	if (SUCCEEDED(hr))
	{
		hr = m_d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dDepthStencilFormat, TRUE, c_d3dMultiSample[desc.multiSample], NULL);
		if (SUCCEEDED(hr))
			d3dMultiSample = c_d3dMultiSample[desc.multiSample];
	}

	std::memset(&d3dPresent, 0, sizeof(d3dPresent));
	d3dPresent.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dPresent.BackBufferCount = 1;
	d3dPresent.BackBufferWidth = rcWindow.right - rcWindow.left;
	d3dPresent.BackBufferHeight = rcWindow.bottom - rcWindow.top;
	d3dPresent.MultiSampleType = d3dMultiSample;
	d3dPresent.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dPresent.hDeviceWindow = (HWND)desc.nativeWindowHandle;
	d3dPresent.Windowed = TRUE;
	d3dPresent.EnableAutoDepthStencil = FALSE;
	d3dPresent.PresentationInterval = desc.waitVBlank ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dPresent.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	Ref< RenderViewWin32 > renderView = new RenderViewWin32(
		this,
		m_parameterCache,
		desc,
		d3dPresent,
		d3dDepthStencilFormat
	);

	hr = renderView->resetDevice(m_d3dDevice);
	if (FAILED(hr))
		return 0;

	return renderView;
}

Ref< VertexBuffer > RenderSystemWin32::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);
	Ref< VertexBufferDx9 > vertexBuffer = new VertexBufferDx9(m_resourceManager, bufferSize, m_vertexDeclCache);
	if (!vertexBuffer->create(m_d3dDevice, vertexElements, dynamic))
		return 0;
	return vertexBuffer;
}

Ref< IndexBuffer > RenderSystemWin32::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);
	Ref< IndexBufferDx9 > indexBuffer = new IndexBufferDx9(m_resourceManager, indexType, bufferSize);
	if (!indexBuffer->create(m_d3dDevice, dynamic))
		return 0;
	return indexBuffer;
}

Ref< ISimpleTexture > RenderSystemWin32::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);
	Ref< SimpleTextureDx9 > texture = new SimpleTextureDx9(m_resourceManager);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

Ref< ICubeTexture > RenderSystemWin32::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);
	Ref< CubeTextureDx9 > texture = new CubeTextureDx9(m_resourceManager);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

Ref< IVolumeTexture > RenderSystemWin32::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);
	Ref< VolumeTextureDx9 > texture = new VolumeTextureDx9(m_resourceManager);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

Ref< RenderTargetSet > RenderSystemWin32::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);
	Ref< RenderTargetSetWin32 > renderTargetSet = new RenderTargetSetWin32(m_resourceManager);
	if (!renderTargetSet->create(m_d3dDevice, desc))
		return 0;
	return renderTargetSet;
}

Ref< IProgram > RenderSystemWin32::createProgram(const ProgramResource* programResource)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);

	Ref< const ProgramResourceDx9 > resource = dynamic_type_cast< const ProgramResourceDx9* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramWin32 > program = new ProgramWin32(m_resourceManager, m_parameterCache);
	if (!program->create(m_d3dDevice, m_shaderCache, resource))
		return 0;

	return program;
}

Ref< IProgramCompiler > RenderSystemWin32::createProgramCompiler() const
{
	return new ProgramCompilerWin32();
}

void RenderSystemWin32::addRenderView(RenderViewWin32* renderView)
{
	m_renderViews.push_back(renderView);
}

void RenderSystemWin32::removeRenderView(RenderViewWin32* renderView)
{
	m_renderViews.remove(renderView);
}

bool RenderSystemWin32::beginRender()
{
	if (!m_renderLock.wait(1000))
		return false;

	if (m_lostDevice)
	{
		Sleep(100);
		return false;
	}

	return true;
}

void RenderSystemWin32::endRender(bool lostDevice)
{
	if (lostDevice)
		m_lostDevice = true;

	m_renderLock.release();
}

void RenderSystemWin32::toggleMode()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);
	HRESULT hr;

	m_d3dPresent.Windowed = !m_d3dPresent.Windowed;
	setWindowStyle(m_hWnd, m_d3dPresent.BackBufferWidth, m_d3dPresent.BackBufferHeight, m_d3dPresent.Windowed ? false : true);

	if (!m_renderViews.empty())
		m_renderViews.front()->setD3DPresent(m_d3dPresent);

	hr = resetDevice();
	if (FAILED(hr))
		m_lostDevice = true;
}

bool RenderSystemWin32::resetPrimary(const D3DPRESENT_PARAMETERS& d3dPresent, D3DFORMAT d3dDepthStencilFormat)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);
	HRESULT hr;

	std::memcpy(&m_d3dPresent, &d3dPresent, sizeof(D3DPRESENT_PARAMETERS));
	setWindowStyle(m_hWnd, m_d3dPresent.BackBufferWidth, m_d3dPresent.BackBufferHeight, m_d3dPresent.Windowed ? false : true);

	// Ensure multi-sample type is supported.
	hr = m_d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_d3dPresent.BackBufferFormat, FALSE, m_d3dPresent.MultiSampleType, NULL);
	if (SUCCEEDED(hr))
	{
		hr = m_d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dDepthStencilFormat, FALSE, m_d3dPresent.MultiSampleType, NULL);
		if (FAILED(hr))
			m_d3dPresent.MultiSampleType = D3DMULTISAMPLE_NONE; 
	}
	else
		m_d3dPresent.MultiSampleType = D3DMULTISAMPLE_NONE;

	// Replace presentation parameters.
	if (!m_renderViews.empty())
		m_renderViews.front()->setD3DPresent(m_d3dPresent);

	// Reset device; recreate all resources.
	hr = resetDevice();
	if (FAILED(hr))
		m_lostDevice = true;

	return true;
}

HRESULT RenderSystemWin32::resetDevice()
{
	HRESULT hr;

	hr = m_resourceManager->lostDevice();
	if (FAILED(hr))
		return hr;

	hr = m_parameterCache->lostDevice();
	if (FAILED(hr))
		return hr;

	hr = m_vertexDeclCache->lostDevice();
	if (FAILED(hr))
		return hr;

	for (RefArray< RenderViewWin32 >::iterator i = m_renderViews.begin(); i != m_renderViews.end(); ++i)
	{
		hr = (*i)->lostDevice();
		if (FAILED(hr))
			return hr;
	}

	hr = m_d3dDevice->Reset(&m_d3dPresent);
	if (FAILED(hr))
		return hr;

	for (RefArray< RenderViewWin32 >::iterator i = m_renderViews.begin(); i != m_renderViews.end(); ++i)
	{
		hr = (*i)->resetDevice(m_d3dDevice);

		if (FAILED(hr))
			return hr;
	}

	hr = m_resourceManager->resetDevice(m_d3dDevice);
	if (FAILED(hr))
		return hr;

	hr = m_parameterCache->resetDevice(m_d3dDevice);
	if (FAILED(hr))
		return hr;

	hr = m_vertexDeclCache->resetDevice(m_d3dDevice);
	if (FAILED(hr))
		return hr;

	for (int i = 0; i < ParameterCache::MaxTextureCount; ++i)
	{
		m_d3dDevice->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *(DWORD*)&m_mipBias);
		m_d3dDevice->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, m_maxAnisotropy);
	}

	log::debug << L"Device reset successful" << Endl;
	return hr;
}

LRESULT RenderSystemWin32::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RenderSystemWin32* renderSystem = reinterpret_cast< RenderSystemWin32* >(GetWindowLongPtr(hWnd, 0));
	LPCREATESTRUCT createStruct;
	LRESULT result = 0;
	
	switch (uMsg)
	{
	case WM_CREATE:
		createStruct = reinterpret_cast< LPCREATESTRUCT >(lParam);
		renderSystem = reinterpret_cast< RenderSystemWin32* >(createStruct->lpCreateParams);
		SetWindowLongPtr(hWnd, 0, reinterpret_cast< LONG_PTR >(renderSystem));
		break;

	case WM_KEYDOWN:
		if (renderSystem && wParam == VK_RETURN)
		{
			if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
				renderSystem->toggleMode();
		}
		break;

	case WM_ACTIVATEAPP:
		if (!wParam)
		{
			// We're about to be deactivated; ensure we're running in windowed mode.
			if (renderSystem && !renderSystem->m_d3dPresent.Windowed)
				renderSystem->toggleMode();
		}
		break;
	
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
		
	case WM_DESTROY:
		SetWindowLongPtr(hWnd, 0, 0);
		PostQuitMessage(0);
		break;
		
	case WM_ERASEBKGND:
		result = TRUE;
		break;

	case WM_SETCURSOR:
		if (renderSystem && !renderSystem->m_d3dPresent.Windowed)
			SetCursor(NULL);
		else
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		result = TRUE;
		break;
	
	default:
		result = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}
	
	return result;
}

	}
}
