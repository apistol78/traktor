#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Thread/Acquire.h"
#include "Render/VertexElement.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Dx9/ClearTarget.h"
#include "Render/Dx9/CubeTextureDx9.h"
#include "Render/Dx9/IndexBufferDx9.h"
#include "Render/Dx9/ParameterCache.h"
#include "Render/Dx9/ProgramResourceDx9.h"
#include "Render/Dx9/ResourceManagerDx9.h"
#include "Render/Dx9/ShaderCache.h"
#include "Render/Dx9/SimpleTextureDx9.h"
#include "Render/Dx9/VertexDeclCache.h"
#include "Render/Dx9/VertexBufferDynamicDx9.h"
#include "Render/Dx9/VertexBufferStaticDx9.h"
#include "Render/Dx9/VolumeTextureDx9.h"
#include "Render/Dx9/Win32/ProgramWin32.h"
#include "Render/Dx9/Win32/ProgramCompilerWin32.h"
#include "Render/Dx9/Win32/RenderSystemWin32.h"
#include "Render/Dx9/Win32/RenderTargetSetWin32.h"
#include "Render/Dx9/Win32/RenderViewDefaultWin32.h"
#include "Render/Dx9/Win32/RenderViewEmbeddedWin32.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemWin32", 0, RenderSystemWin32, IRenderSystem)

RenderSystemWin32::RenderSystemWin32()
:	m_vertexDeclCache(0)
,	m_maxAnisotropy(0)
,	m_inRender(false)
,	m_deviceLost(false)
{
}

bool RenderSystemWin32::create(const RenderSystemDesc& desc)
{
	UINT d3dAdapter = D3DADAPTER_DEFAULT;
	D3DDEVTYPE d3dDevType = D3DDEVTYPE_HAL;
	D3DCAPS9 d3dCaps;
	HRESULT hr;

	m_d3d.getAssign() = Direct3DCreate9(D3D_SDK_VERSION);
	T_ASSERT (m_d3d);

	if (FAILED(m_d3d->GetAdapterDisplayMode(d3dAdapter, &m_d3dDefaultDisplayMode)))
		return false;

	std::memset(&d3dCaps, 0, sizeof(d3dCaps));
	hr = m_d3d->GetDeviceCaps(d3dAdapter, d3dDevType, &d3dCaps);
	if (SUCCEEDED(hr))
	{
		if (d3dCaps.VertexShaderVersion > D3DVS_VERSION(0, 0) && d3dCaps.VertexShaderVersion < D3DVS_VERSION(3, 0))
		{
			log::error << L"Out dated shader model; need at least VS 3.0 (device VS " << uint32_t(D3DSHADER_VERSION_MAJOR(d3dCaps.VertexShaderVersion)) << L"." << uint32_t(D3DSHADER_VERSION_MINOR(d3dCaps.VertexShaderVersion)) << L")" << Endl;
			return false;
		}
		if (d3dCaps.PixelShaderVersion < D3DPS_VERSION(3, 0))
		{
			log::error << L"Out dated shader model; need at least PS 3.0 (device PS " << uint32_t(D3DSHADER_VERSION_MAJOR(d3dCaps.PixelShaderVersion)) << L"." << uint32_t(D3DSHADER_VERSION_MINOR(d3dCaps.PixelShaderVersion)) << L")" << Endl;
			return false;
		}
	}
	else
		log::warning << L"Unable to get device capabilities (" << int32_t(hr) << L"); may cause unexpected behavior" << Endl;
	
	// Render window class.
	m_window = new Window();
	if (!m_window->create())
		return false;

	DWORD dwBehaviour = D3DCREATE_MULTITHREADED;
	if (d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		T_DEBUG(L"Using D3DCREATE_HARDWARE_VERTEXPROCESSING");
		dwBehaviour |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}

	std::memset(&m_d3dPresent, 0, sizeof(m_d3dPresent));
	m_d3dPresent.BackBufferFormat = D3DFMT_UNKNOWN;
	m_d3dPresent.BackBufferCount = 1;
	m_d3dPresent.BackBufferWidth = 1;
	m_d3dPresent.BackBufferHeight = 1;
	m_d3dPresent.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_d3dPresent.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dPresent.hDeviceWindow = *m_window;
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
		log::warning << L"CreateDevice failed; trying D3DCREATE_SOFTWARE_VERTEXPROCESSING..." << Endl;

		dwBehaviour &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
		dwBehaviour |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

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
			log::error << L"CreateDevice failed; unable to create device (" << int32_t(hr) << L")" << Endl;
			return false;
		}
	}

	m_resourceManager = new ResourceManagerDx9();
	m_shaderCache = new ShaderCache();
	m_parameterCache = new ParameterCache(m_d3dDevice, desc.mipBias, desc.maxAnisotropy);
	m_vertexDeclCache = new VertexDeclCache(m_d3dDevice);
	m_maxAnisotropy = desc.maxAnisotropy;

	m_clearTarget = new ClearTarget();
	if (!m_clearTarget->create(m_d3dDevice))
		return false;

#if defined(T_USE_RENDER_MARKERS)
	log::warning << L"Render markers enabled; should be disabled for RTM builds" << Endl;
#endif

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

	if (m_clearTarget)
		m_clearTarget = 0;

	m_d3dDevice.release();
	m_d3d.release();
}

bool RenderSystemWin32::reset(const RenderSystemDesc& desc)
{
	m_parameterCache = new ParameterCache(m_d3dDevice, desc.mipBias, desc.maxAnisotropy);
	m_maxAnisotropy = desc.maxAnisotropy;
	return true;
}

void RenderSystemWin32::getInformation(RenderSystemInformation& outInfo) const
{
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

float RenderSystemWin32::getDisplayAspectRatio() const
{
	return float(m_d3dDefaultDisplayMode.Width) / m_d3dDefaultDisplayMode.Height;
}

Ref< IRenderView > RenderSystemWin32::createRenderView(const RenderViewDefaultDesc& desc)
{
	if (!m_renderViews.empty())
		return 0;

	Ref< RenderViewWin32 > renderView = new RenderViewDefaultWin32(
		this,
		m_clearTarget,
		m_parameterCache,
		m_d3dDevice,
		m_d3d,
		m_d3dPresent,
		m_window
	);

	if (!renderView->reset(desc))
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

	d3dDepthStencilFormat = determineDepthStencilFormat(m_d3d, desc.depthBits, desc.stencilBits, D3DFMT_X8R8G8B8);
	if (d3dDepthStencilFormat == D3DFMT_UNKNOWN)
	{
		log::error << L"Render view failed; any of the required depth/stencil formats supported" << Endl;
		return 0;
	}

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

	Ref< RenderViewWin32 > renderView = new RenderViewEmbeddedWin32(
		this,
		m_clearTarget,
		m_parameterCache,
		m_d3dDevice,
		d3dPresent,
		d3dDepthStencilFormat
	);

	hr = renderView->resetDevice();
	if (FAILED(hr))
		return 0;

	return renderView;
}

Ref< VertexBuffer > RenderSystemWin32::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);

	if (!dynamic)
	{
		Ref< VertexBufferStaticDx9 > vertexBuffer = new VertexBufferStaticDx9(bufferSize, m_vertexDeclCache);
		if (vertexBuffer->create(m_d3dDevice, vertexElements))
			return vertexBuffer;
	}
	else
	{
		Ref< VertexBufferDynamicDx9 > vertexBuffer = new VertexBufferDynamicDx9(m_resourceManager, bufferSize, m_vertexDeclCache);
		if (vertexBuffer->create(m_d3dDevice, vertexElements))
			return vertexBuffer;
	}

	return 0;
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
	if (!program->create(m_d3dDevice, m_shaderCache, resource, m_maxAnisotropy))
		return 0;

	return program;
}

Ref< IProgramCompiler > RenderSystemWin32::createProgramCompiler() const
{
	return new ProgramCompilerWin32();
}

void RenderSystemWin32::getStatistics(RenderSystemStatistics& outStatistics) const
{
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
	T_ASSERT (!m_inRender);

	if (!m_renderLock.wait(1000))
		return false;

	if (!tryRecoverDevice())
	{
		m_renderLock.release();
		return false;
	}

	m_inRender = true;
	return true;
}

void RenderSystemWin32::endRender(bool deviceLost)
{
	T_ASSERT (m_inRender);

	m_inRender = false;
	m_deviceLost = deviceLost;
	m_renderLock.release();
}

bool RenderSystemWin32::tryRecoverDevice()
{
	HRESULT hr;

	if (!m_deviceLost)
		return true;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_renderLock);

		hr = m_d3dDevice->TestCooperativeLevel();
		if (hr == D3DERR_DEVICENOTRESET)
			hr = resetDevice();

		if (SUCCEEDED(hr))
		{
			m_deviceLost = false;
			return true;
		}
	}

	return false;
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
		hr = (*i)->resetDevice();

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

	T_DEBUG(L"Device reset successful");
	return hr;
}

	}
}
