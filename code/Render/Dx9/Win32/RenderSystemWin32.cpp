#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Thread/Acquire.h"
#include "Render/DisplayMode.h"
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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemWin32", 0, RenderSystemWin32, IRenderSystem)

RenderSystemWin32::RenderSystemWin32()
:	m_vertexDeclCache(0)
,	m_hWnd(0)
,	m_mipBias(0.0f)
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
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}

	m_d3dDevice.release();
	m_d3d.release();
}

int RenderSystemWin32::getDisplayModeCount() const
{
	return int(m_d3d->GetAdapterModeCount(D3DADAPTER_DEFAULT, m_d3dDefaultDisplayMode.Format));
}

Ref< DisplayMode > RenderSystemWin32::getDisplayMode(int index)
{
	D3DDISPLAYMODE dm;
	
	if (FAILED(m_d3d->EnumAdapterModes(D3DADAPTER_DEFAULT, m_d3dDefaultDisplayMode.Format, index, &dm)))
		return 0;
	
	return new DisplayMode(
		index,
		dm.Width,
		dm.Height,
		0
	);
}

Ref< DisplayMode > RenderSystemWin32::getCurrentDisplayMode()
{
	return new DisplayMode(
		0,
		m_d3dDefaultDisplayMode.Width,
		m_d3dDefaultDisplayMode.Height,
		0
	);
}

bool RenderSystemWin32::handleMessages()
{
	bool going = true;
	MSG msg;
	
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret <= 0 || msg.message == WM_QUIT)
			going = false;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return going;
}

Ref< IRenderView > RenderSystemWin32::createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc)
{
	D3DFORMAT d3dDepthStencilFormat;
	D3DMULTISAMPLE_TYPE d3dMultiSample;
	HRESULT hr;

	T_ASSERT (m_hWnd);
	T_ASSERT (m_renderViews.empty());
	T_ASSERT (displayMode);

	// Determine output aspect ratio from default display mode; not
	// correct but we assume user have a matching resolution and monitor
	// have square pixels.
	float aspectRatio = float(m_d3dDefaultDisplayMode.Width) / m_d3dDefaultDisplayMode.Height;

	SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, displayMode->getWidth(), displayMode->getHeight(), SWP_SHOWWINDOW);
	ShowWindow(m_hWnd, SW_MAXIMIZE);
	UpdateWindow(m_hWnd);

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
	m_d3dPresent.BackBufferWidth = displayMode->getWidth();
	m_d3dPresent.BackBufferHeight = displayMode->getHeight();
	m_d3dPresent.MultiSampleType = d3dMultiSample;
	m_d3dPresent.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dPresent.hDeviceWindow = m_hWnd;
	m_d3dPresent.Windowed = FALSE;
	m_d3dPresent.EnableAutoDepthStencil = FALSE;
	m_d3dPresent.PresentationInterval = desc.waitVBlank ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	m_d3dPresent.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	hr = m_d3dDevice->Reset(&m_d3dPresent);
	if (FAILED(hr))
		return 0;

	Ref< RenderViewWin32 > renderView = new RenderViewWin32(
		this,
		m_parameterCache,
		desc,
		m_d3dPresent,
		d3dDepthStencilFormat,
		aspectRatio
	);

	hr = renderView->resetDevice(m_d3dDevice);
	if (FAILED(hr))
		return 0;

	return renderView;
}

Ref< IRenderView > RenderSystemWin32::createRenderView(void* windowHandle, const RenderViewCreateDesc& desc)
{
	D3DPRESENT_PARAMETERS d3dPresent;
	D3DFORMAT d3dDepthStencilFormat;
	D3DMULTISAMPLE_TYPE d3dMultiSample;
	HRESULT hr;
	RECT rcWindow;

	// Determine output aspect ratio from default display mode; not
	// correct but we assume user have a matching resolution and monitor
	// have square pixels.
	float aspectRatio = float(m_d3dDefaultDisplayMode.Width) / m_d3dDefaultDisplayMode.Height;

	GetClientRect((HWND)windowHandle, &rcWindow);
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
	d3dPresent.hDeviceWindow = (HWND)windowHandle;
	d3dPresent.Windowed = TRUE;
	d3dPresent.EnableAutoDepthStencil = FALSE;
	d3dPresent.PresentationInterval = desc.waitVBlank ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dPresent.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	Ref< RenderViewWin32 > renderView = new RenderViewWin32(
		this,
		m_parameterCache,
		desc,
		d3dPresent,
		d3dDepthStencilFormat,
		aspectRatio
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
	HRESULT hr;

	if (!m_renderLock.wait(1000))
		return false;

	if (m_lostDevice)
	{
		hr = m_d3dDevice->TestCooperativeLevel();

		if (hr == D3DERR_DEVICENOTRESET)
			hr = resetDevice();

		if (SUCCEEDED(hr))
			m_lostDevice = false;
		else
		{
			Sleep(100);
			return false;
		}
	}

	return true;
}

void RenderSystemWin32::endRender(bool lostDevice)
{
	m_lostDevice = lostDevice;
	m_renderLock.release();
}

void RenderSystemWin32::toggleMode()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);

	if (m_d3dPresent.Windowed)
	{
		log::debug << L"Enter fullscreen mode" << Endl;

		SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUPWINDOW);
		SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		ShowWindow(m_hWnd, SW_MAXIMIZE);
		UpdateWindow(m_hWnd);

		m_d3dPresent.Windowed = FALSE;
	}
	else
	{
		log::debug << L"Enter windowed mode" << Endl;

		SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX);
		SetWindowPos(m_hWnd, HWND_TOP, 64, 64, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_SHOWWINDOW);
		ShowWindow(m_hWnd, SW_NORMAL);
		UpdateWindow(m_hWnd);

		RECT rcWindow;
		GetClientRect(m_hWnd, &rcWindow);

		int32_t frameWidth = 2 * m_d3dPresent.BackBufferWidth - (rcWindow.right - rcWindow.left);
		int32_t frameHeight = 2 * m_d3dPresent.BackBufferHeight - (rcWindow.bottom - rcWindow.top);

		log::debug << L"Window size " << frameWidth << L"x" << frameHeight << Endl;

		SetWindowPos(m_hWnd, NULL, 0, 0, frameWidth, frameHeight, SWP_NOMOVE | SWP_NOZORDER);

		m_d3dPresent.Windowed = TRUE;
	}

	resetDevice();
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

	hr = m_d3dDevice->EvictManagedResources();
	if (FAILED(hr))
		return hr;

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
		m_d3dDevice->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *(DWORD*)&m_mipBias);

	log::debug << L"Device reset successful" << Endl;
	return hr;
}

LRESULT RenderSystemWin32::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RenderSystemWin32* renderSystem = reinterpret_cast< RenderSystemWin32* >(GetWindowLongPtr(hWnd, 0));
	LPCREATESTRUCT createStruct;
	LRESULT result = TRUE;
	
	switch (uMsg)
	{
	case WM_CREATE:
		createStruct = reinterpret_cast< LPCREATESTRUCT >(lParam);
		renderSystem = reinterpret_cast< RenderSystemWin32* >(createStruct->lpCreateParams);
		SetWindowLongPtr(hWnd, 0, reinterpret_cast< LONG_PTR >(renderSystem));
		break;

	case WM_SYSKEYDOWN:
		if (wParam == VK_RETURN && (lParam & (1 << 29)) != 0)
			renderSystem->toggleMode();
		break;

#if 0
	case WM_KEYDOWN:
		if (wParam != VK_ESCAPE)
			break;
#endif
	
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
		
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		
	case WM_ERASEBKGND:
		break;

	case WM_SETCURSOR:
		if (!renderSystem->m_d3dPresent.Windowed)
			SetCursor(NULL);
		break;
	
	default:
		result = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}
	
	return result;
}

	}
}
