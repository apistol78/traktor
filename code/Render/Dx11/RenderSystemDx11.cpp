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
#include "Render/Dx11/ResourceCache.h"
#include "Render/Dx11/SimpleTextureDx11.h"
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
{
}

bool RenderSystemDx11::create(const RenderSystemDesc& desc)
{
	ComRef< ID3D11Device > d3dDevice;
	ComRef< ID3D11DeviceContext > d3dDeviceContext;
	ComRef< IDXGIDevice1 > dxgiDevice;
	ComRef< IDXGIAdapter1 > dxgiAdapterEnum;
	ComRef< IDXGIAdapter1 > dxgiAdapter;
	ComRef< IDXGIFactory1 > dxgiFactory;
	ComRef< IDXGIOutput > dxgiOutput;
	D3D_FEATURE_LEVEL d3dFeatureLevel;
	HRESULT hr;

	// Create DXGI factory.
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory.getAssign());
	if (FAILED(hr))
	{
		log::warning << L"Failed to create DXGI factory, HRESULT " << int32_t(hr) << Endl;
		log::warning << L"Unable to enumerate preferred adapter; using DX default" << Endl;
		dxgiFactory.release();
	}

	if (dxgiFactory)
	{
		// Prefer AMD or NVidia adapters; if none is found fallback on all others.
		for (int32_t i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapterEnum.getAssign()) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			// Ensure the adapter have a connected output.
			hr = dxgiAdapterEnum->EnumOutputs(0, &dxgiOutput.getAssign());
			if (FAILED(hr))
				continue;

			// Get adapter description.
			DXGI_ADAPTER_DESC1 dad;

			hr = dxgiAdapterEnum->GetDesc1(&dad);
			if (FAILED(hr))
				continue;

			if (desc.verbose)
				log::info << L"Device " << (i + 1) << L" \"" << dad.Description << L"\" (" << dad.DeviceId << L")" << Endl;

			if (dad.VendorId == 4098)	// AMD/ATI
				dxgiAdapter = dxgiAdapterEnum;
			if (dad.VendorId == 4318)	// NVidia
				dxgiAdapter = dxgiAdapterEnum;
		}

		// In case we didn't find an suitable adapter we need to get the factory
		// determined by DX itself.
		if (!dxgiAdapter)
		{
			log::warning << L"No preferred DX adapter found; using DX default" << Endl;
			dxgiFactory.release();
		}
	}

	// Create D3D11 device instance.
	hr = D3D11CreateDevice(
		dxgiAdapter,
		dxgiAdapter != 0 ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		0,
		0,
		D3D11_SDK_VERSION,
		&d3dDevice.getAssign(),
		&d3dFeatureLevel,
		&d3dDeviceContext.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Failed to create DirectX 11 device, HRESULT " << int32_t(hr) << Endl;
		return false;
	}

	hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), (void **)&dxgiDevice.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Failed to get IDXGIDevice1 interface, HRESULT " << int32_t(hr) << Endl;
		return false;
	}

	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter1), (void **)&dxgiAdapter.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Failed to get IDXGIAdapter1 interface, HRESULT " << int32_t(hr) << Endl;
		return false;
	}

	DXGI_ADAPTER_DESC1 dad;
	hr = dxgiAdapter->GetDesc1(&dad);
	if (SUCCEEDED(hr))
	{
		if (dad.VendorId == 0 && dad.DeviceId == 0)
		{
			log::error << L"Failed to get supported DirectX 11 adapter" << Endl;
			return false;
		}

		if (desc.verbose)
		{
			log::info << L"Using DirectX 11 adapter:" << Endl;
			log::info << IncreaseIndent;
			log::info << L"Description " << dad.Description << Endl;
			log::info << L"VendorId " << dad.VendorId << Endl;
			log::info << L"DeviceId " << dad.DeviceId << Endl;
			log::info << L"SubSysId " << dad.SubSysId << Endl;
			log::info << L"Revision " << dad.Revision << Endl;
			log::info << L"DedicatedVideoMemory " << uint64_t(dad.DedicatedVideoMemory / (1024*1024)) << L" MiB" << Endl;
			log::info << L"DedicatedSystemMemory " << uint64_t(dad.DedicatedSystemMemory / (1024*1024)) << L" MiB" << Endl;
			log::info << L"SharedSystemMemory " << uint64_t(dad.SharedSystemMemory / (1024*1024)) << L" MiB" << Endl;
			log::info << DecreaseIndent;
		}
	}
	else
		log::warning << L"Unable to get DirectX 11 adapter description, HRESULT " << int32_t(hr) << Endl;

	if (!dxgiFactory)
	{
		hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), (void **)&dxgiFactory.getAssign());
		if (FAILED(hr))
		{
			log::error << L"Failed to get IDXGIFactory1 interface, HRESULT " << int32_t(hr) << Endl;
			return false;
		}
	}

	hr = dxgiAdapter->EnumOutputs(0, &dxgiOutput.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Failed to enumerate DXGI outputs, HRESULT " << int32_t(hr) << Endl;
		return false;
	}

	// Do not allow more than 2 frames of latency.
	hr = dxgiDevice->SetMaximumFrameLatency(2);
	if (FAILED(hr))
		log::warning << L"Unable to cap frame latency on DXGI output" << Endl;

	DisplayMode dm = getCurrentDisplayMode();
	m_displayAspect = float(dm.width) / dm.height;

	m_window = new Window();
	if (!m_window->create())
	{
		log::error << L"Failed to create render window" << Endl;
		return false;
	}

	m_context = new ContextDx11(d3dDevice, d3dDeviceContext, dxgiFactory, dxgiOutput);
	m_resourceCache = new ResourceCache(d3dDevice, desc.mipBias, clamp(desc.maxAnisotropy, 1, 16));

	return true;
}

void RenderSystemDx11::destroy()
{
	m_resourceCache = 0;
	if (m_context)
	{
		m_context->deleteResources();
		m_context = 0;
	}
}

bool RenderSystemDx11::reset(const RenderSystemDesc& desc)
{
	m_resourceCache->reset(desc.mipBias, clamp(desc.maxAnisotropy, 1, 16));
	return true;
}

void RenderSystemDx11::getInformation(RenderSystemInformation& outInfo) const
{
	ComRef< IDXGIDevice1 > dxgiDevice;
	ComRef< IDXGIAdapter1 > dxgiAdapter;
	DXGI_ADAPTER_DESC1 desc;
	HRESULT hr;

	hr = m_context->getD3DDevice()->QueryInterface(__uuidof(IDXGIDevice1), (void **)&dxgiDevice.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Failed to get IDXGIDevice1 interface, HRESULT " << int32_t(hr) << Endl;
		return;
	}

	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter1), (void **)&dxgiAdapter.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Failed to get IDXGIAdapter1 interface, HRESULT " << int32_t(hr) << Endl;
		return;
	}

	hr = dxgiAdapter->GetDesc1(&desc);
	if (FAILED(hr))
	{
		log::error << L"Failed to get IDXGIAdapter1 description, HRESULT " << int32_t(hr) << Endl;
		return;
	}

	outInfo.dedicatedMemoryTotal = desc.DedicatedVideoMemory;
	outInfo.sharedMemoryTotal = desc.SharedSystemMemory;
	outInfo.dedicatedMemoryAvailable = 0;
	outInfo.sharedMemoryAvailable = 0;
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

	if (!setupSampleDesc(m_context->getD3DDevice(), desc.multiSample, scd.BufferDesc.Format, DXGI_FORMAT_D24_UNORM_S8_UINT, scd.SampleDesc))
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
	if (!dynamic)
		return VertexBufferStaticDx11::create(m_context, bufferSize, vertexElements);
	else
		return VertexBufferDynamicDx11::create(m_context, bufferSize, vertexElements);
}

Ref< IndexBuffer > RenderSystemDx11::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	if (!dynamic)
		return IndexBufferStaticDx11::create(m_context, indexType, bufferSize);
	else
		return IndexBufferDynamicDx11::create(m_context, indexType, bufferSize);
}

Ref< ISimpleTexture > RenderSystemDx11::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureDx11 > texture = new SimpleTextureDx11(m_context);
	if (!texture->create(desc))
		return 0;
	return texture;
}

Ref< ICubeTexture > RenderSystemDx11::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTextureDx11 > texture = new CubeTextureDx11(m_context);
	if (!texture->create(desc))
		return 0;
	return texture;
}

Ref< IVolumeTexture > RenderSystemDx11::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	Ref< VolumeTextureDx11 > texture = new VolumeTextureDx11(m_context);
	if (!texture->create(desc))
		return 0;
	return texture;
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
	Ref< const ProgramResourceDx11 > resource = dynamic_type_cast< const ProgramResourceDx11* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramDx11 > program = new ProgramDx11(m_context);
	if (!program->create(m_context->getD3DDevice(), *m_resourceCache, resource))
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
