#include "Core/Log/Log.h"
#include "Render/VertexElement.h"
#include "Render/Dx10/RenderSystemDx10.h"
#include "Render/Dx10/RenderViewDx10.h"
#include "Render/Dx10/ContextDx10.h"
#include "Render/Dx10/VertexBufferDx10.h"
#include "Render/Dx10/IndexBufferDx10.h"
#include "Render/Dx10/SimpleTextureDx10.h"
#include "Render/Dx10/CubeTextureDx10.h"
#include "Render/Dx10/VolumeTextureDx10.h"
#include "Render/Dx10/RenderTargetSetDx10.h"
#include "Render/Dx10/ProgramCompilerDx10.h"
#include "Render/Dx10/ProgramResourceDx10.h"
#include "Render/Dx10/ProgramDx10.h"
#include "Render/Dx10/TypesDx10.h"
#include "Render/Dx10/Utilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemDx10", 0, RenderSystemDx10, IRenderSystem)

RenderSystemDx10::RenderSystemDx10()
:	m_displayAspect(0.0f)
,	m_mipBias(0.0f)
{
}

bool RenderSystemDx10::create(const RenderSystemDesc& desc)
{
	ComRef< ID3D10Device > d3dDevice;
	ComRef< IDXGIFactory > dxgiFactory;
	ComRef< IDXGIDevice > dxgiDevice;
	ComRef< IDXGIAdapter > dxgiAdapter;
	ComRef< IDXGIOutput > dxgiOutput;
	HRESULT hr;

	hr = D3D10CreateDevice(
		NULL,
		D3D10_DRIVER_TYPE_HARDWARE,
		NULL,
#if defined(_DEBUG)
		D3D10_CREATE_DEVICE_DEBUG,
#else
		0,
#endif
		D3D10_SDK_VERSION,
		&d3dDevice.getAssign()
	);
	if (FAILED(hr))
		return false;

	hr = CreateDXGIFactory(
		__uuidof(IDXGIFactory),
		(void**)(&dxgiFactory.getAssign())
	);
	if (FAILED(hr))
		return false;

	hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice.getAssign());
	if (FAILED(hr))
		return 0;

	hr = dxgiDevice->GetAdapter(&dxgiAdapter.getAssign());
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

	m_context = new ContextDx10(d3dDevice, dxgiFactory, dxgiOutput);
	m_mipBias = desc.mipBias;

	return true;
}

void RenderSystemDx10::destroy()
{
	if (m_context)
	{
		m_context->deleteResources();
		m_context = 0;
	}
}

bool RenderSystemDx10::reset(const RenderSystemDesc& desc)
{
	m_mipBias = desc.mipBias;
	return true;
}

void RenderSystemDx10::getInformation(RenderSystemInformation& outInfo) const
{
	ComRef< IDXGIDevice > dxgiDevice;
	ComRef< IDXGIAdapter > dxgiAdapter;
	DXGI_ADAPTER_DESC desc;
	HRESULT hr;

	hr = m_context->getD3DDevice()->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Failed to get IDXGIDevice1 interface, HRESULT " << int32_t(hr) << Endl;
		return;
	}

	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&dxgiAdapter.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Failed to get IDXGIAdapter1 interface, HRESULT " << int32_t(hr) << Endl;
		return;
	}

	hr = dxgiAdapter->GetDesc(&desc);
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

uint32_t RenderSystemDx10::getDisplayModeCount() const
{
	return traktor::render::getDisplayModeCount(m_context->getDXGIOutput());
}

DisplayMode RenderSystemDx10::getDisplayMode(uint32_t index) const
{
	DisplayMode dm;
	traktor::render::getDisplayMode(m_context->getDXGIOutput(), index, dm);
	return dm;
}

DisplayMode RenderSystemDx10::getCurrentDisplayMode() const
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

float RenderSystemDx10::getDisplayAspectRatio() const
{
	return m_displayAspect;
}

Ref< IRenderView > RenderSystemDx10::createRenderView(const RenderViewDefaultDesc& desc)
{
	Ref< RenderViewDx10 > renderView = new RenderViewDx10(m_context, m_window);

	if (!renderView->reset(desc))
		return 0;

	return renderView;
}

Ref< IRenderView > RenderSystemDx10::createRenderView(const RenderViewEmbeddedDesc& desc)
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

	Ref< RenderViewDx10 > renderView = new RenderViewDx10(m_context, dxgiSwapChain);

	if (!renderView->reset(scd.BufferDesc.Width, scd.BufferDesc.Height))
		return 0;

	return renderView;
}

Ref< VertexBuffer > RenderSystemDx10::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	ComRef< ID3D10Buffer > d3dBuffer;
	D3D10_BUFFER_DESC dbd;
	HRESULT hr;

	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D10_USAGE_DYNAMIC;
	dbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	dbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	dbd.MiscFlags = 0;

	hr = m_context->getD3DDevice()->CreateBuffer(&dbd, NULL, &d3dBuffer.getAssign());
	if (FAILED(hr))
		return 0;

	std::vector< D3D10_INPUT_ELEMENT_DESC > d3dInputElements(vertexElements.size());
	for (unsigned int i = 0; i < vertexElements.size(); ++i)
	{
		T_ASSERT (vertexElements[i].getDataUsage() < sizeof_array(c_dxgiInputSemantic));
		T_ASSERT (vertexElements[i].getDataType() < sizeof_array(c_dxgiInputType));

		d3dInputElements[i].SemanticName = c_dxgiInputSemantic[vertexElements[i].getDataUsage()];
		d3dInputElements[i].SemanticIndex = vertexElements[i].getIndex();
		d3dInputElements[i].Format = c_dxgiInputType[vertexElements[i].getDataType()];
		d3dInputElements[i].InputSlot = 0;
		d3dInputElements[i].AlignedByteOffset = vertexElements[i].getOffset();
		d3dInputElements[i].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
		d3dInputElements[i].InstanceDataStepRate = 0;
	}

	return new VertexBufferDx10(
		m_context,
		bufferSize,
		d3dBuffer,
		getVertexSize(vertexElements),
		d3dInputElements
	);
}

Ref< IndexBuffer > RenderSystemDx10::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	ComRef< ID3D10Buffer > d3dBuffer;
	D3D10_BUFFER_DESC dbd;
	HRESULT hr;

	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D10_USAGE_DYNAMIC;
	dbd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	dbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	dbd.MiscFlags = 0;

	hr = m_context->getD3DDevice()->CreateBuffer(&dbd, NULL, &d3dBuffer.getAssign());
	if (FAILED(hr))
		return 0;

	return new IndexBufferDx10(m_context, indexType, bufferSize, d3dBuffer);
}

Ref< ISimpleTexture > RenderSystemDx10::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureDx10 > texture = new SimpleTextureDx10(m_context);
	if (!texture->create(m_context->getD3DDevice(), desc))
		return 0;
	return texture;
}

Ref< ICubeTexture > RenderSystemDx10::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTextureDx10 > texture = new CubeTextureDx10(m_context);
	if (!texture->create(m_context->getD3DDevice(), desc))
		return 0;
	return texture;
}

Ref< IVolumeTexture > RenderSystemDx10::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	return new VolumeTextureDx10();
}

Ref< RenderTargetSet > RenderSystemDx10::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetDx10 > renderTargetSet = new RenderTargetSetDx10(m_context);
	if (!renderTargetSet->create(m_context->getD3DDevice(), desc))
		return 0;
	return renderTargetSet;
}

Ref< IProgram > RenderSystemDx10::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	Ref< const ProgramResourceDx10 > resource = dynamic_type_cast< const ProgramResourceDx10* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramDx10 > program = new ProgramDx10(m_context);
	if (!program->create(m_context->getD3DDevice(), resource, m_mipBias))
		return 0;

	return program;
}

Ref< IProgramCompiler > RenderSystemDx10::createProgramCompiler() const
{
	return new ProgramCompilerDx10();
}

Ref< ITimeQuery > RenderSystemDx10::createTimeQuery() const
{
	return 0;
}

void RenderSystemDx10::purge()
{
}

void RenderSystemDx10::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

	}
}
