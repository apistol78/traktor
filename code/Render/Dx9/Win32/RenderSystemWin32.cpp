#include "Render/Dx9/Win32/RenderSystemWin32.h"
#include "Render/Dx9/Win32/RenderViewWin32.h"
#include "Render/Dx9/Win32/RenderTargetSetWin32.h"
#include "Render/Dx9/Win32/ProgramCompilerWin32.h"
#include "Render/Dx9/Win32/ProgramWin32.h"
#include "Render/Dx9/ContextDx9.h"
#include "Render/Dx9/ShaderCache.h"
#include "Render/Dx9/ParameterCache.h"
#include "Render/Dx9/VertexDeclCache.h"
#include "Render/Dx9/ProgramResourceDx9.h"
#include "Render/Dx9/VertexBufferDx9.h"
#include "Render/Dx9/IndexBufferDx9.h"
#include "Render/Dx9/SimpleTextureDx9.h"
#include "Render/Dx9/CubeTextureDx9.h"
#include "Render/Dx9/VolumeTextureDx9.h"
#include "Render/Dx9/Unmanaged.h"
#include "Render/DisplayMode.h"
#include "Render/VertexElement.h"
#include "Render/ShaderGraph.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Acquire.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"

#define T_ENABLE_NVIDIA_PERHUD 0

namespace traktor
{
	namespace render
	{
		namespace
		{

const TCHAR* c_className = _T("TraktorRenderSystem");

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemWin32", 0, RenderSystemWin32, IRenderSystem)

uint32_t RenderSystemWin32::ms_instances = 0;

RenderSystemWin32::RenderSystemWin32()
:	m_parameterCache(0)
,	m_vertexDeclCache(0)
,	m_hWnd(0)
{
}

bool RenderSystemWin32::create()
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

#if T_ENABLE_NVIDIA_PERHUD
	for (UINT adapter = 0; adapter < m_d3d->GetAdapterCount(); ++adapter)
	{
		D3DADAPTER_IDENTIFIER9 d3did;
		
		hr = m_d3d->GetAdapterIdentifier(adapter, 0, &d3did);
		if (strstr(d3did.Description, "PerfHUD") != 0)
		{
			log::debug << L"Found PerfHUD device!" << Endl;
			d3dAdapter = adapter;
			d3dDevType = D3DDEVTYPE_REF;
			break;
		}
	}
#endif

	// Create "resource" device.
	DWORD dwBehaviour =
		D3DCREATE_HARDWARE_VERTEXPROCESSING |
		D3DCREATE_FPU_PRESERVE |
		D3DCREATE_MULTITHREADED |
		D3DCREATE_PUREDEVICE;

	D3DPRESENT_PARAMETERS d3dPresentNull;

	std::memset(&d3dPresentNull, 0, sizeof(d3dPresentNull));
	d3dPresentNull.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dPresentNull.BackBufferCount = 1;
	d3dPresentNull.BackBufferWidth = 1;
	d3dPresentNull.BackBufferHeight = 1;
	d3dPresentNull.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dPresentNull.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dPresentNull.hDeviceWindow = m_hWnd;
	d3dPresentNull.Windowed = TRUE;
	d3dPresentNull.EnableAutoDepthStencil = FALSE;
	d3dPresentNull.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dPresentNull.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	hr = m_d3d->CreateDevice(
		d3dAdapter,
		d3dDevType,
		d3dPresentNull.hDeviceWindow,
		dwBehaviour,
		&d3dPresentNull,
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
	log::debug << L"Estimated " << availTextureMem / (1024 * 1024) << L" Mb texture memory available" << Endl;

	T_ASSERT (!m_shaderCache);
	m_shaderCache= new ShaderCache();

	T_ASSERT (!m_parameterCache);
	m_parameterCache = new ParameterCache(this, m_d3dDevice);

	T_ASSERT (!m_vertexDeclCache);
	m_vertexDeclCache = new VertexDeclCache(this, m_d3dDevice);

	m_context = new ContextDx9();
	ms_instances++;
	return true;
}

void RenderSystemWin32::destroy()
{
	T_ASSERT (m_renderViews.empty());

	// Ensure all DX9 objects are properly collected.
	if (--ms_instances == 0)
	{
		//Heap::collectAllOf(type_of< ProgramWin32 >());
		//Heap::collectAllOf(type_of< VertexBufferDx9 >());
		//Heap::collectAllOf(type_of< IndexBufferDx9 >());
		//Heap::collectAllOf(type_of< SimpleTextureDx9 >());
		//Heap::collectAllOf(type_of< CubeTextureDx9 >());
		//Heap::collectAllOf(type_of< VolumeTextureDx9 >());
	}

	// In case there are any unmanaged resources still lingering we tell them to get lost.
	for (std::list< Unmanaged* >::iterator i = m_unmanagedList.begin(); i != m_unmanagedList.end(); ++i)
	{
		if (*i)
			(*i)->lostDevice();
	}

	if (m_context)
	{
		m_context->deleteResources();
		m_context = 0;
	}

	if (m_shaderCache)
	{
		m_shaderCache->releaseAll();
		m_shaderCache = 0;
	}

	if (m_parameterCache)
	{
		delete m_parameterCache;
		m_parameterCache = 0;
	}

	if (m_vertexDeclCache)
	{
		delete m_vertexDeclCache;
		m_vertexDeclCache = 0;
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
	D3DPRESENT_PARAMETERS d3dPresent;
	D3DFORMAT d3dDepthStencilFormat;

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

	std::memset(&d3dPresent, 0, sizeof(d3dPresent));
	d3dPresent.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dPresent.BackBufferCount = 1;
	d3dPresent.BackBufferWidth = displayMode->getWidth();
	d3dPresent.BackBufferHeight = displayMode->getHeight();
	d3dPresent.MultiSampleType = c_d3dMultiSample[desc.multiSample];
	d3dPresent.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dPresent.hDeviceWindow = m_hWnd;
	d3dPresent.Windowed = FALSE;
	d3dPresent.EnableAutoDepthStencil = FALSE;
	d3dPresent.PresentationInterval = desc.waitVBlank ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dPresent.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	if (desc.stencilBits == 1)
		d3dDepthStencilFormat = D3DFMT_D15S1;
	else if (desc.stencilBits > 1)
		d3dDepthStencilFormat = D3DFMT_D24S8;
	else
		d3dDepthStencilFormat = (desc.depthBits > 16) ? D3DFMT_D24X8 : D3DFMT_D16;

	return new RenderViewWin32(
		m_context,
		m_parameterCache,
		desc,
		this,
		d3dPresent,
		d3dDepthStencilFormat,
		aspectRatio
	);
}

Ref< IRenderView > RenderSystemWin32::createRenderView(void* windowHandle, const RenderViewCreateDesc& desc)
{
	D3DPRESENT_PARAMETERS d3dPresent;
	D3DFORMAT d3dDepthStencilFormat;
	RECT rcWindow;

	T_ASSERT (m_hWnd);

	// Determine output aspect ratio from default display mode; not
	// correct but we assume user have a matching resolution and monitor
	// have square pixels.
	float aspectRatio = float(m_d3dDefaultDisplayMode.Width) / m_d3dDefaultDisplayMode.Height;

	GetClientRect((HWND)windowHandle, &rcWindow);
	if (rcWindow.left >= rcWindow.right)
		rcWindow.right = rcWindow.left + 10;
	if (rcWindow.top >= rcWindow.bottom)
		rcWindow.bottom = rcWindow.top + 10;

	std::memset(&d3dPresent, 0, sizeof(d3dPresent));
	d3dPresent.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dPresent.BackBufferCount = 1;
	d3dPresent.BackBufferWidth = rcWindow.right - rcWindow.left;
	d3dPresent.BackBufferHeight = rcWindow.bottom - rcWindow.top;
	d3dPresent.MultiSampleType = c_d3dMultiSample[desc.multiSample];
	d3dPresent.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dPresent.hDeviceWindow = (HWND)windowHandle;
	d3dPresent.Windowed = TRUE;
	d3dPresent.EnableAutoDepthStencil = FALSE;
	d3dPresent.PresentationInterval = desc.waitVBlank ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dPresent.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	if (desc.stencilBits == 1)
		d3dDepthStencilFormat = D3DFMT_D15S1;
	else if (desc.stencilBits > 1)
		d3dDepthStencilFormat = D3DFMT_D24S8;
	else
		d3dDepthStencilFormat = (desc.depthBits > 16) ? D3DFMT_D24X8 : D3DFMT_D16;

	return new RenderViewWin32(
		m_context,
		m_parameterCache,
		desc,
		this,
		d3dPresent,
		d3dDepthStencilFormat,
		aspectRatio
	);
}

Ref< VertexBuffer > RenderSystemWin32::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	Ref< VertexBufferDx9 > vertexBuffer = new VertexBufferDx9(this, m_context, bufferSize, m_vertexDeclCache);
	if (!vertexBuffer->create(m_d3dDevice, vertexElements, dynamic))
		return 0;
	return vertexBuffer;
}

Ref< IndexBuffer > RenderSystemWin32::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	Ref< IndexBufferDx9 > indexBuffer = new IndexBufferDx9(this, m_context, indexType, bufferSize);
	if (!indexBuffer->create(m_d3dDevice, dynamic))
		return 0;
	return indexBuffer;
}

Ref< ISimpleTexture > RenderSystemWin32::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureDx9 > texture = new SimpleTextureDx9(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

Ref< ICubeTexture > RenderSystemWin32::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTextureDx9 > texture = new CubeTextureDx9(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

Ref< IVolumeTexture > RenderSystemWin32::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	Ref< VolumeTextureDx9 > texture = new VolumeTextureDx9(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

Ref< RenderTargetSet > RenderSystemWin32::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetWin32 > renderTargetSet = new RenderTargetSetWin32(this, m_context);
	if (!renderTargetSet->create(m_d3dDevice, desc))
		return 0;
	return renderTargetSet;
}

Ref< IProgram > RenderSystemWin32::createProgram(const ProgramResource* programResource)
{
	T_ASSERT (m_shaderCache);
	T_ASSERT (m_parameterCache);

	Ref< const ProgramResourceDx9 > resource = dynamic_type_cast< const ProgramResourceDx9* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramWin32 > program = new ProgramWin32(this, m_context, m_shaderCache, m_parameterCache);
	if (!program->create(m_d3dDevice, resource))
		return 0;

	return program;
}

Ref< IProgramCompiler > RenderSystemWin32::createProgramCompiler() const
{
	return new ProgramCompilerWin32();
}

IDirect3D9* RenderSystemWin32::getD3D() const
{
	return m_d3d;
}

IDirect3DDevice9* RenderSystemWin32::getD3DDevice() const
{
	return m_d3dDevice;
}

HRESULT RenderSystemWin32::testCooperativeLevel()
{
	HRESULT hr;

	hr = m_d3dDevice->TestCooperativeLevel();
	if (hr == D3DERR_DEVICENOTRESET)
		hr = resetDevice();

	return hr;
}

HRESULT RenderSystemWin32::resetDevice()
{
	HRESULT hr;

	// Release all unmanaged resources.
	{
		Acquire< Semaphore > lock(m_unmanagedLock);
		for (std::list< Unmanaged* >::iterator i = m_unmanagedList.begin(); i != m_unmanagedList.end(); ++i)
			(*i)->lostDevice();
	}

	// Delete any pending resources.
	m_context->deleteResources();

	RefArray< RenderViewWin32 >::iterator i = m_renderViews.begin();
	if (i == m_renderViews.end())
		return S_OK;

	D3DPRESENT_PARAMETERS d3dPresent = (*i)->getD3DPresent();
	if (d3dPresent.Windowed)
	{
		hr = m_d3dDevice->Reset(&d3dPresent);
		if (FAILED(hr))
		{
			log::error << L"Reset device failed; unable to continue" << Endl;
			return hr;
		}

		// Create additional swap chains for all windowed render views.
		for (; i != m_renderViews.end(); ++i)
		{
			RenderViewWin32* renderView = *i;
			if (!renderView)
				continue;

			D3DPRESENT_PARAMETERS d3dPresent = renderView->getD3DPresent();
			T_ASSERT (d3dPresent.Windowed);

			ComRef< IDirect3DSwapChain9 > d3dSwapChain;
			hr = m_d3dDevice->CreateAdditionalSwapChain(
				&d3dPresent,
				&d3dSwapChain.getAssign()
			);
			if (FAILED(hr))
			{
				log::error << L"Reset device failed, unable to create additional swap chain" << Endl;
				return hr;
			}

			ComRef< IDirect3DSurface9 > d3dDepthStencilSurface;
			hr = m_d3dDevice->CreateDepthStencilSurface(
				d3dPresent.BackBufferWidth,
				d3dPresent.BackBufferHeight,
				renderView->getD3DDepthStencilFormat(),
				d3dPresent.MultiSampleType,
				0,
				TRUE,
				&d3dDepthStencilSurface.getAssign(),
				NULL
			);
			if (FAILED(hr))
			{
				log::error << L"Reset device failed, unable to create additional depth/stencil surface" << Endl;
				return hr;
			}

			renderView->setD3DBuffers(d3dSwapChain, d3dDepthStencilSurface);
		}
	}
	else
	{
		T_ASSERT (m_renderViews.size() == 1);

		hr = m_d3dDevice->Reset(&d3dPresent);
		if (FAILED(hr))
		{
			log::error << L"Reset device failed, unable to continue" << Endl;
			return hr;
		}

		ComRef< IDirect3DSwapChain9 > d3dSwapChain;
		hr = m_d3dDevice->GetSwapChain(
			0,
			&d3dSwapChain.getAssign()
		);
		if (FAILED(hr))
		{
			log::error << L"Reset device failed, unable to get primary swap chain" << Endl;
			return hr;
		}

		ComRef< IDirect3DSurface9 > d3dDepthStencilSurface;
		hr = m_d3dDevice->CreateDepthStencilSurface(
			d3dPresent.BackBufferWidth,
			d3dPresent.BackBufferHeight,
			(*i)->getD3DDepthStencilFormat(),
			d3dPresent.MultiSampleType,
			0,
			TRUE,
			&d3dDepthStencilSurface.getAssign(),
			NULL
		);
		if (FAILED(hr))
		{
			log::error << L"Reset device failed, unable to create depth/stencil surface" << Endl;
			return hr;
		}

		(*i)->setD3DBuffers(d3dSwapChain, d3dDepthStencilSurface);

		// Disable screen saver and power-save feature.
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, 0, SPIF_SENDWININICHANGE);
		SystemParametersInfo(SPI_SETPOWEROFFACTIVE, FALSE, 0, SPIF_SENDWININICHANGE);
	}

	if (SUCCEEDED(hr))
	{
		// Evict all managed resources.
		hr = m_d3dDevice->EvictManagedResources();
		if (FAILED(hr))
		{
			log::error << L"Reset device failed, unable to evict managed resources" << Endl;
			return hr;
		}

		// Reset unmanaged resources.
		{
			Acquire< Semaphore > lock(m_unmanagedLock);
			for (std::list< Unmanaged* >::iterator i = m_unmanagedList.begin(); i != m_unmanagedList.end(); ++i)
				(*i)->resetDevice(m_d3dDevice);
		}

		// Set default mip bias.
		for (int i = 0; i < 8; ++i)
		{
			const float c_mipBias = -1.0f;
			m_d3dDevice->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *(DWORD*)&c_mipBias);
		}
	}

	return hr;
}

void RenderSystemWin32::addRenderView(RenderViewWin32* renderView)
{
	m_renderViews.push_back(renderView);
	HRESULT hr = resetDevice();
	T_ASSERT (SUCCEEDED(hr));
}

void RenderSystemWin32::removeRenderView(RenderViewWin32* renderView)
{
	m_renderViews.remove(renderView);
	HRESULT hr = resetDevice();
	T_ASSERT (SUCCEEDED(hr));
}

void RenderSystemWin32::addUnmanaged(Unmanaged* unmanaged)
{
	Acquire< Semaphore > lock(m_unmanagedLock);
	m_unmanagedList.push_back(unmanaged);
}

void RenderSystemWin32::removeUnmanaged(Unmanaged* unmanaged)
{
	Acquire< Semaphore > lock(m_unmanagedLock);
	m_unmanagedList.remove(unmanaged);
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
