#include "Render/Dx10/RenderSystemDx10.h"
#include "Render/Dx10/RenderViewDx10.h"
#include "Render/Dx10/ContextDx10.h"
#include "Render/Dx10/VertexBufferDx10.h"
#include "Render/Dx10/IndexBufferDx10.h"
#include "Render/Dx10/SimpleTextureDx10.h"
#include "Render/Dx10/CubeTextureDx10.h"
#include "Render/Dx10/VolumeTextureDx10.h"
#include "Render/Dx10/RenderTargetSetDx10.h"
#include "Render/Dx10/ProgramResourceDx10.h"
#include "Render/Dx10/ProgramDx10.h"
#include "Render/Dx10/Hlsl.h"
#include "Render/Dx10/HlslProgram.h"
#include "Render/Dx10/TypesDx10.h"
#include "Render/VertexElement.h"
#include "Core/Misc/ComRef.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.RenderSystemDx10", RenderSystemDx10, RenderSystem)

bool RenderSystemDx10::create()
{
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
		&m_d3dDevice.getAssign()
	);
	if (FAILED(hr))
		return false;

	hr = CreateDXGIFactory(
		__uuidof(IDXGIFactory),
		(void**)(&m_dxgiFactory.getAssign())
	);
	if (FAILED(hr))
		return false;

	m_context = gc_new< ContextDx10 >();

	return true;
}

void RenderSystemDx10::destroy()
{
	m_dxgiFactory.release();
	m_d3dDevice.release();

	if (m_context)
	{
		m_context->deleteResources();
		m_context = 0;
	}
}

int RenderSystemDx10::getDisplayModeCount() const
{
	return 0;
}

DisplayMode* RenderSystemDx10::getDisplayMode(int index)
{
	return 0;
}

DisplayMode* RenderSystemDx10::getCurrentDisplayMode()
{
	return 0;
}

bool RenderSystemDx10::handleMessages()
{
	return false;
}

RenderView* RenderSystemDx10::createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc)
{
	return 0;
}

RenderView* RenderSystemDx10::createRenderView(void* windowHandle, const RenderViewCreateDesc& desc)
{
	ComRef< IDXGISwapChain > d3dSwapChain;
	HRESULT hr;

	RECT rc;
	GetClientRect((HWND)windowHandle, &rc);

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
	scd.OutputWindow = (HWND)windowHandle;
	scd.Windowed = TRUE;

	if (desc.multiSample > 0)
	{
        scd.SampleDesc.Count = 0;
        for (uint32_t i = 1; i <= D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT; ++i)
        {
            UINT msQuality1 = 0;
            UINT msQuality2 = 0;

			hr = m_d3dDevice->CheckMultisampleQualityLevels(
				scd.BufferDesc.Format,
				i,
				&msQuality1
			);

            if (SUCCEEDED(hr) && msQuality1 > 0)
            {
				hr = m_d3dDevice->CheckMultisampleQualityLevels(
					DXGI_FORMAT_D16_UNORM,
					i,
					&msQuality2
				);

                if (SUCCEEDED(hr) && msQuality2 > 0)
				{
	                scd.SampleDesc.Count = i;
		            scd.SampleDesc.Quality = min(msQuality1 - 1, msQuality2 - 1);
				}
            }
        }
		if (!scd.SampleDesc.Count)
			return 0;
	}

	hr = m_dxgiFactory->CreateSwapChain(
		m_d3dDevice,
		&scd,
		&d3dSwapChain.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"CreateSwapChain failed" << Endl;
		return 0;
	}

	return gc_new< RenderViewDx10 >(m_context, m_d3dDevice, d3dSwapChain, scd);
}

VertexBuffer* RenderSystemDx10::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	ComRef< ID3D10Buffer > d3dBuffer;
	D3D10_BUFFER_DESC dbd;
	HRESULT hr;

	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D10_USAGE_DYNAMIC;
	dbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	dbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	dbd.MiscFlags = 0;

	hr = m_d3dDevice->CreateBuffer(&dbd, NULL, &d3dBuffer.getAssign());
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

	return gc_new< VertexBufferDx10 >(
		m_context,
		bufferSize,
		d3dBuffer,
		getVertexSize(vertexElements),
		cref(d3dInputElements)
	);
}

IndexBuffer* RenderSystemDx10::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	ComRef< ID3D10Buffer > d3dBuffer;
	D3D10_BUFFER_DESC dbd;
	HRESULT hr;

	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D10_USAGE_DYNAMIC;
	dbd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	dbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	dbd.MiscFlags = 0;

	hr = m_d3dDevice->CreateBuffer(&dbd, NULL, &d3dBuffer.getAssign());
	if (FAILED(hr))
		return 0;

	return gc_new< IndexBufferDx10 >(m_context, indexType, bufferSize, d3dBuffer);
}

SimpleTexture* RenderSystemDx10::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureDx10 > texture = gc_new< SimpleTextureDx10 >(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

CubeTexture* RenderSystemDx10::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTextureDx10 > texture = gc_new< CubeTextureDx10 >(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

VolumeTexture* RenderSystemDx10::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	return gc_new< VolumeTextureDx10 >();
}

RenderTargetSet* RenderSystemDx10::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetDx10 > renderTargetSet = gc_new< RenderTargetSetDx10 >(m_context);
	if (!renderTargetSet->create(m_d3dDevice, desc))
		return 0;
	return renderTargetSet;
}

ProgramResource* RenderSystemDx10::compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate)
{
	HlslProgram hlslProgram;
	if (!Hlsl().generate(shaderGraph, hlslProgram))
		return 0;

	Ref< ProgramResourceDx10 > programResource = ProgramDx10::compile(m_d3dDevice, hlslProgram);
	if (!programResource)
		return 0;

	return programResource;
}

Program* RenderSystemDx10::createProgram(const ProgramResource* programResource)
{
	Ref< const ProgramResourceDx10 > resource = dynamic_type_cast< const ProgramResourceDx10* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramDx10 > program = gc_new< ProgramDx10 >(m_context);
	if (!program->create(m_d3dDevice, resource))
		return 0;

	return program;
}

	}
}
