#include "Render/Dx9/Win32/RenderSystemWin32.h"
#include "Render/Dx9/Win32/RenderViewWin32.h"
#include "Render/Dx9/Win32/RenderTargetSetWin32.h"
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
#include "Render/Dx9/Hlsl.h"
#include "Render/Dx9/HlslProgram.h"
#include "Render/Dx9/Unmanaged.h"
#include "Render/DisplayMode.h"
#include "Render/VertexElement.h"
#include "Render/ShaderGraph.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const TCHAR* c_className = _T("TraktorRenderSystem");

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.RenderSystemWin32", RenderSystemWin32, IRenderSystem)

RenderSystemWin32::RenderSystemWin32()
:	m_parameterCache(0)
,	m_vertexDeclCache(0)
,	m_hWnd(0)
{
}

bool RenderSystemWin32::create()
{
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

	m_context = gc_new< ContextDx9 >();
	return true;
}

void RenderSystemWin32::destroy()
{
	T_ASSERT (m_renderViews.empty());
	T_ASSERT (m_unmanagedList.empty());

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

DisplayMode* RenderSystemWin32::getDisplayMode(int index)
{
	D3DDISPLAYMODE dm;
	
	if (FAILED(m_d3d->EnumAdapterModes(D3DADAPTER_DEFAULT, m_d3dDefaultDisplayMode.Format, index, &dm)))
		return 0;
	
	return gc_new< DisplayMode >(
		index,
		dm.Width,
		dm.Height,
		0
	);
}

DisplayMode* RenderSystemWin32::getCurrentDisplayMode()
{
	return gc_new< DisplayMode >(
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

IRenderView* RenderSystemWin32::createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc)
{
	D3DPRESENT_PARAMETERS d3dPresent;
	D3DFORMAT d3dDepthStencilFormat;

	T_ASSERT (m_hWnd);
	T_ASSERT (!m_d3dDevice);
	T_ASSERT (m_renderViews.empty());
	T_ASSERT (displayMode);

	SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, displayMode->getWidth(), displayMode->getHeight(), SWP_SHOWWINDOW);
	ShowWindow(m_hWnd, SW_MAXIMIZE);
	UpdateWindow(m_hWnd);

	std::memset(&d3dPresent, 0, sizeof(d3dPresent));
	d3dPresent.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dPresent.BackBufferCount = 1;
	d3dPresent.BackBufferWidth = displayMode->getWidth();
	d3dPresent.BackBufferHeight = displayMode->getHeight();
	d3dPresent.MultiSampleType = (D3DMULTISAMPLE_TYPE)desc.multiSample;
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

	return gc_new< RenderViewWin32 >(
		m_context,
		cref(desc),
		this,
		d3dPresent,
		d3dDepthStencilFormat
	);
}

IRenderView* RenderSystemWin32::createRenderView(void* windowHandle, const RenderViewCreateDesc& desc)
{
	D3DPRESENT_PARAMETERS d3dPresent;
	D3DFORMAT d3dDepthStencilFormat;
	RECT rcWindow;

	T_ASSERT (m_hWnd);

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
	d3dPresent.MultiSampleType = (D3DMULTISAMPLE_TYPE)desc.multiSample;
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

	return gc_new< RenderViewWin32 >(
		m_context,
		cref(desc),
		this,
		d3dPresent,
		d3dDepthStencilFormat
	);
}

VertexBuffer* RenderSystemWin32::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	Ref< VertexBufferDx9 > vertexBuffer = gc_new< VertexBufferDx9 >(this, m_context, bufferSize, m_vertexDeclCache);
	if (!vertexBuffer->create(m_d3dDevice, vertexElements, dynamic))
		return 0;
	return vertexBuffer;
}

IndexBuffer* RenderSystemWin32::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	Ref< IndexBufferDx9 > indexBuffer = gc_new< IndexBufferDx9 >(this, m_context, indexType, bufferSize);
	if (!indexBuffer->create(m_d3dDevice, dynamic))
		return 0;
	return indexBuffer;
}

ISimpleTexture* RenderSystemWin32::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureDx9 > texture = gc_new< SimpleTextureDx9 >(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

ICubeTexture* RenderSystemWin32::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTextureDx9 > texture = gc_new< CubeTextureDx9 >(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

IVolumeTexture* RenderSystemWin32::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	Ref< VolumeTextureDx9 > texture = gc_new< VolumeTextureDx9 >(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

RenderTargetSet* RenderSystemWin32::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetWin32 > renderTargetSet = gc_new< RenderTargetSetWin32 >(this, m_context);
	if (!renderTargetSet->create(m_d3dDevice, desc))
		return 0;
	return renderTargetSet;
}

ProgramResource* RenderSystemWin32::compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate)
{
	HlslProgram program;
	if (!Hlsl().generate(shaderGraph, program))
		return 0;

	Ref< ProgramResourceDx9 > programResource = ProgramWin32::compile(program, optimize, validate);
	if (!programResource)
		return 0;

	return programResource;
}

IProgram* RenderSystemWin32::createProgram(const ProgramResource* programResource)
{
	T_ASSERT (m_shaderCache);
	T_ASSERT (m_parameterCache);

	Ref< const ProgramResourceDx9 > resource = dynamic_type_cast< const ProgramResourceDx9* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramWin32 > program = gc_new< ProgramWin32 >(this, m_context, m_shaderCache, m_parameterCache);
	if (!program->create(m_d3dDevice, resource))
		return 0;

	return program;
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

	RefList< RenderViewWin32 >::iterator i = m_renderViews.begin();
	if (i == m_renderViews.end())
	{
		// We've no render views, collect all resources and release device if any.
		Heap& heap = Heap::getInstance();

		heap.collectAllOf(type_of< VertexBufferDx9 >());
		heap.collectAllOf(type_of< IndexBufferDx9 >());
		heap.collectAllOf(type_of< ProgramWin32 >());
		heap.collectAllOf(type_of< SimpleTextureDx9 >());
		heap.collectAllOf(type_of< CubeTextureDx9 >());
		heap.collectAllOf(type_of< VolumeTextureDx9 >());
		heap.collectAllOf(type_of< RenderTargetSetWin32 >());

		m_d3dDevice.release();

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

		// Delete any pending resources.
		m_context->deleteResources();

		return S_OK;
	}

	// Delete any pending resources.
	m_context->deleteResources();

	D3DPRESENT_PARAMETERS d3dPresent = (*i)->getD3DPresent();
	if (d3dPresent.Windowed)
	{
		if (!m_d3dDevice)
		{
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
			d3dPresentNull.MultiSampleType = (D3DMULTISAMPLE_TYPE)0;
			d3dPresentNull.SwapEffect = D3DSWAPEFFECT_DISCARD;
			d3dPresentNull.hDeviceWindow = m_hWnd;
			d3dPresentNull.Windowed = TRUE;
			d3dPresentNull.EnableAutoDepthStencil = FALSE;
			d3dPresentNull.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
			d3dPresentNull.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

			hr = m_d3d->CreateDevice(
				0,
				D3DDEVTYPE_HAL,
				d3dPresentNull.hDeviceWindow,
				dwBehaviour,
				&d3dPresentNull,
				&m_d3dDevice.getAssign()
			);
			if (FAILED(hr) || !m_d3dDevice)
			{
				log::error << L"Reset device failed, unable to create device" << Endl;
				return hr;
			}

			D3DCAPS9 d3dDeviceCaps;
			std::memset(&d3dDeviceCaps, 0, sizeof(d3dDeviceCaps));
			hr = m_d3dDevice->GetDeviceCaps(&d3dDeviceCaps);
			if (SUCCEEDED(hr))
			{
				// Ensure device supports at least SM 3.0.
				if (d3dDeviceCaps.VertexShaderVersion < D3DVS_VERSION(3, 0))
				{
					log::error << L"Reset device failed, need atleast VS 3.0 (device VS " << uint32_t(D3DSHADER_VERSION_MAJOR(d3dDeviceCaps.VertexShaderVersion)) << L"." << uint32_t(D3DSHADER_VERSION_MINOR(d3dDeviceCaps.VertexShaderVersion)) << L")" << Endl;
					return S_FALSE;
				}
				if (d3dDeviceCaps.PixelShaderVersion < D3DPS_VERSION(3, 0))
				{
					log::error << L"Reset device failed, need atleast PS 3.0 (device PS " << uint32_t(D3DSHADER_VERSION_MAJOR(d3dDeviceCaps.PixelShaderVersion)) << L"." << uint32_t(D3DSHADER_VERSION_MINOR(d3dDeviceCaps.PixelShaderVersion)) << L")" << Endl;
					return S_FALSE;
				}

				log::debug << L"D3DCAPS9.MaxVertexShaderConst = " << uint32_t(d3dDeviceCaps.MaxVertexShaderConst) << Endl;
			}
			else
				log::warning << L"Unable to get device capabilities; may produce unexpected results" << Endl;

			T_ASSERT (!m_shaderCache);
			m_shaderCache= gc_new< ShaderCache >();

			T_ASSERT (!m_parameterCache);
			m_parameterCache = new ParameterCache(this, m_d3dDevice);

			T_ASSERT (!m_vertexDeclCache);
			m_vertexDeclCache = new VertexDeclCache(this, m_d3dDevice);
		}
		else
		{
			hr = m_d3dDevice->Reset(&d3dPresent);
			if (FAILED(hr))
			{
				log::error << L"Reset device failed, unable to reset" << Endl;
				return hr;
			}
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

		if (!m_d3dDevice)
		{
			DWORD dwBehaviour =
				D3DCREATE_HARDWARE_VERTEXPROCESSING |
				D3DCREATE_FPU_PRESERVE |
				D3DCREATE_MULTITHREADED |
				D3DCREATE_PUREDEVICE;

			hr = m_d3d->CreateDevice(
				0,
				D3DDEVTYPE_HAL,
				d3dPresent.hDeviceWindow,
				dwBehaviour,
				&d3dPresent,
				&m_d3dDevice.getAssign()
			);
			if (FAILED(hr) || !m_d3dDevice)
			{
				log::error << L"Reset device failed, unable to create device" << Endl;
				return hr;
			}

			T_ASSERT (!m_shaderCache);
			m_shaderCache = gc_new< ShaderCache >();

			T_ASSERT (!m_parameterCache);
			m_parameterCache = new ParameterCache(this, m_d3dDevice);
		}
		else
		{
			hr = m_d3dDevice->Reset(&d3dPresent);
			if (FAILED(hr))
			{
				log::error << L"Reset device failed, unable to reset" << Endl;
				return hr;
			}
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
	
	case WM_KEYDOWN:
		if (wParam != VK_ESCAPE)
			break;
	
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
