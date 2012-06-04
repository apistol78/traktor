#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/CubeTextureDx11.h"
#include "Render/Dx11/IndexBufferDynamicDx11.h"
#include "Render/Dx11/IndexBufferStaticDx11.h"
#include "Render/Dx11/ProgramDx11.h"
#include "Render/Dx11/ProgramCompilerDx11.h"
#include "Render/Dx11/ProgramResourceDx11.h"
#include "Render/Dx11/RenderSystemDx11.h"
#include "Render/Dx11/RenderTargetSetDx11.h"
#include "Render/Dx11/RenderViewDx11.h"
#include "Render/Dx11/SimpleTextureDx11.h"
#include "Render/Dx11/StateCache.h"
#include "Render/Dx11/TypesDx11.h"
#include "Render/Dx11/Utilities.h"
#include "Render/Dx11/VertexBufferDynamicDx11.h"
#include "Render/Dx11/VertexBufferStaticDx11.h"
#include "Render/Dx11/VolumeTextureDx11.h"
#include "Render/Dx11/Window.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemDx11", 0, RenderSystemDx11, IRenderSystem)

RenderSystemDx11::RenderSystemDx11()
:	m_displayAspect(0.0f)
,	m_mipBias(0.0f)
,	m_maxAnisotropy(1)
{
}

bool RenderSystemDx11::create(const RenderSystemCreateDesc& desc)
{
	ComRef< ID3D11Device > d3dDevice;
	ComRef< ID3D11DeviceContext > d3dDeviceContext;
	ComRef< IDXGIDevice1 > dxgiDevice;
	ComRef< IDXGIAdapter1 > dxgiAdapter;
	ComRef< IDXGIFactory1 > dxgiFactory;
	ComRef< IDXGIOutput > dxgiOutput;
	D3D_FEATURE_LEVEL d3dFeatureLevel;
	HRESULT hr;

	// Create D3D11 device instance.
	hr = D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
//#if defined(_DEBUG)
//		D3D11_CREATE_DEVICE_DEBUG,
//#else
		0,
//#endif
		0,
		0,
		D3D11_SDK_VERSION,
		&d3dDevice.getAssign(),
		&d3dFeatureLevel,
		&d3dDeviceContext.getAssign()
	);
	if (FAILED(hr))
		return false;

	hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), (void **)&dxgiDevice.getAssign());
	if (FAILED(hr))
		return 0;

	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter1), (void **)&dxgiAdapter.getAssign());
	if (FAILED(hr))
		return 0;

	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), (void **)&dxgiFactory.getAssign());
	if (FAILED(hr))
		return 0;

	hr = dxgiAdapter->EnumOutputs(0, &dxgiOutput.getAssign());
	if (FAILED(hr))
		return 0;

	DisplayMode dm = getCurrentDisplayMode();
	m_displayAspect = float(dm.width) / dm.height;

	m_window = new Window();
	if (!m_window->create())
		return 0;

	m_context = new ContextDx11(d3dDevice, d3dDeviceContext, dxgiFactory, dxgiOutput);
	m_stateCache = new StateCache(d3dDevice);

	m_mipBias = desc.mipBias;
	m_maxAnisotropy = clamp(desc.maxAnisotropy, 1, 16);
	return true;
}

void RenderSystemDx11::destroy()
{
	m_stateCache = 0;

	if (m_context)
	{
		m_context->deleteResources();
		m_context = 0;
	}
}

uint32_t RenderSystemDx11::getDisplayModeCount() const
{
	return traktor::render::getDisplayModeCount(m_context->getDXGIOutput());
}

DisplayMode RenderSystemDx11::getDisplayMode(uint32_t index) const
{
	DisplayMode dm;
	traktor::render::getDisplayMode(m_context->getDXGIOutput(), index, dm);
	return dm;
}

DisplayMode RenderSystemDx11::getCurrentDisplayMode() const
{
	DEVMODE dm;

	std::memset(&dm, 0, sizeof(dm));
	dm.dmSize = sizeof(dm);

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

	DisplayMode cdm;

	cdm.width = dm.dmPelsWidth;
	cdm.height = dm.dmPelsHeight;
	cdm.refreshRate = uint16_t(dm.dmDisplayFrequency);
	cdm.colorBits = uint16_t(dm.dmBitsPerPel);
	cdm.stereoscopic = false;

	return cdm;
}

float RenderSystemDx11::getDisplayAspectRatio() const
{
	return m_displayAspect;
}

Ref< IRenderView > RenderSystemDx11::createRenderView(const RenderViewDefaultDesc& desc)
{
	Ref< RenderViewDx11 > renderView = new RenderViewDx11(m_context, m_window);

	if (!renderView->reset(desc))
		return 0;

	return renderView;
}

Ref< IRenderView > RenderSystemDx11::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	ComRef< IDXGISwapChain > dxgiSwapChain;
	HRESULT hr;

	RECT rc;
	GetClientRect((HWND)desc.nativeWindowHandle, &rc);

	if (rc.right <= rc.left)
		rc.right = 8;
	if (rc.bottom <= rc.top)
		rc.bottom = 8;

	DXGI_SWAP_CHAIN_DESC scd;
	std::memset(&scd, 0, sizeof(scd));
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.BufferCount = 1;
	scd.BufferDesc.Width = rc.right - rc.left;
	scd.BufferDesc.Height = rc.bottom - rc.top;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = (HWND)desc.nativeWindowHandle;
	scd.Windowed = TRUE;

	if (!setupSampleDesc(m_context->getD3DDevice(), desc.multiSample, scd.BufferDesc.Format, DXGI_FORMAT_D16_UNORM, scd.SampleDesc))
	{
		log::error << L"Unable to create render view; unsupported MSAA" << Endl;
		return 0;
	}

	hr = m_context->getDXGIFactory()->CreateSwapChain(
		m_context->getD3DDevice(),
		&scd,
		&dxgiSwapChain.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; CreateSwapChain failed" << Endl;
		return 0;
	}

	m_context->getDXGIFactory()->MakeWindowAssociation(
		scd.OutputWindow,
		DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER
	);

	Ref< RenderViewDx11 > renderView = new RenderViewDx11(m_context, dxgiSwapChain);

	if (!renderView->reset(scd.BufferDesc.Width, scd.BufferDesc.Height))
		return 0;

	return renderView;
}

Ref< VertexBuffer > RenderSystemDx11::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	if (!dynamic)
		return VertexBufferStaticDx11::create(m_context, bufferSize, vertexElements);
	else
		return VertexBufferDynamicDx11::create(m_context, bufferSize, vertexElements);
}

Ref< IndexBuffer > RenderSystemDx11::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	if (!dynamic)
		return IndexBufferStaticDx11::create(m_context, indexType, bufferSize);
	else
		return IndexBufferDynamicDx11::create(m_context, indexType, bufferSize);
}

Ref< ISimpleTexture > RenderSystemDx11::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	Ref< SimpleTextureDx11 > texture = new SimpleTextureDx11(m_context);
	if (!texture->create(desc))
		return 0;
	return texture;
}

Ref< ICubeTexture > RenderSystemDx11::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	Ref< CubeTextureDx11 > texture = new CubeTextureDx11(m_context);
	if (!texture->create(desc))
		return 0;
	return texture;
}

Ref< IVolumeTexture > RenderSystemDx11::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	return new VolumeTextureDx11();
}

Ref< RenderTargetSet > RenderSystemDx11::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	Ref< RenderTargetSetDx11 > renderTargetSet = new RenderTargetSetDx11(m_context);
	if (!renderTargetSet->create(m_context->getD3DDevice(), desc))
		return 0;
	return renderTargetSet;
}

Ref< IProgram > RenderSystemDx11::createProgram(const ProgramResource* programResource)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());

	Ref< const ProgramResourceDx11 > resource = dynamic_type_cast< const ProgramResourceDx11* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramDx11 > program = new ProgramDx11(m_context);
	if (!program->create(m_context->getD3DDevice(), *m_stateCache, resource, m_mipBias, m_maxAnisotropy))
		return 0;

	return program;
}

Ref< IProgramCompiler > RenderSystemDx11::createProgramCompiler() const
{
	return new ProgramCompilerDx11();
}

void RenderSystemDx11::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

	}
}
