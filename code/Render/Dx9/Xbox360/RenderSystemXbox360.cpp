#include "Render/Dx9/Xbox360/RenderSystemXbox360.h"
#include "Render/Dx9/Xbox360/RenderViewXbox360.h"
#include "Render/Dx9/Xbox360/RenderTargetPool.h"
#include "Render/Dx9/Xbox360/RenderTargetSetXbox360.h"
#include "Render/Dx9/Xbox360/ProgramXbox360.h"
#include "Render/Dx9/ContextDx9.h"
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
#include "Render/Dx9/TypesDx9.h"
#include "Render/DisplayMode.h"
#include "Render/VertexElement.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.RenderSystemXbox360", RenderSystemXbox360, IRenderSystem)

RenderSystemXbox360::RenderSystemXbox360()
:	m_parameterCache(0)
,	m_vertexDeclCache(0)
{
}

bool RenderSystemXbox360::create()
{
	m_d3d = Direct3DCreate9(D3D_SDK_VERSION);
	return m_d3d != 0;
}

void RenderSystemXbox360::destroy()
{
	//if (m_renderTargetPool)
	//{
	//	m_renderTargetPool->destroy(m_d3dDevice);
	//	m_renderTargetPool = 0;
	//}

	if (m_context)
	{
		m_context->deleteResources();
		m_context = 0;
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
}

int RenderSystemXbox360::getDisplayModeCount() const
{
	return 1;
}

DisplayMode* RenderSystemXbox360::getDisplayMode(int index)
{
	return gc_new< DisplayMode >(
		0,
		1280,
		720,
		0
	);
}

DisplayMode* RenderSystemXbox360::getCurrentDisplayMode()
{
	return gc_new< DisplayMode >(
		0,
		1280,
		720,
		0
	);
}

bool RenderSystemXbox360::handleMessages()
{
	return true;
}

IRenderView* RenderSystemXbox360::createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc)
{
	D3DPRESENT_PARAMETERS d3dPresent;
	D3DFORMAT d3dDepthStencilFormat;
	HRESULT hr;

	T_ASSERT (!m_renderView);
	T_ASSERT (displayMode);

	memset(&d3dPresent, 0, sizeof(d3dPresent));
	d3dPresent.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dPresent.BackBufferCount = 0;
	d3dPresent.BackBufferWidth = displayMode->getWidth();
	d3dPresent.BackBufferHeight = displayMode->getHeight();
	d3dPresent.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dPresent.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dPresent.hDeviceWindow = NULL;
	d3dPresent.Windowed = FALSE;
	d3dPresent.EnableAutoDepthStencil = FALSE;
	d3dPresent.DisableAutoBackBuffer  = TRUE;
	d3dPresent.DisableAutoFrontBuffer = TRUE;
	d3dPresent.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dPresent.PresentationInterval = desc.waitVBlank ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dPresent.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dPresent.RingBufferParameters.PrimarySize = 0;
	d3dPresent.RingBufferParameters.SecondarySize = 6 * 1024 * 1024;

	if (desc.stencilBits > 0)
		d3dDepthStencilFormat = D3DFMT_D24S8;
	else
		d3dDepthStencilFormat = (desc.depthBits > 16) ? D3DFMT_D24X8 : D3DFMT_D16;

	hr = m_d3d->CreateDevice(
		0,
		D3DDEVTYPE_HAL,
		NULL,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_BUFFER_2_FRAMES,
		(D3DPRESENT_PARAMETERS*)&d3dPresent,
		&m_d3dDevice.getAssign()
	);
	T_ASSERT (SUCCEEDED(hr));

	m_context = gc_new< ContextDx9 >();
	m_parameterCache = new ParameterCache(this, m_d3dDevice);
	m_vertexDeclCache = new VertexDeclCache(this, m_d3dDevice);

	m_renderTargetPool = gc_new< RenderTargetPool >();
	m_renderView = gc_new< RenderViewXbox360 >(
		desc,
		this,
		m_d3dDevice,
		m_renderTargetPool,
		displayMode->getWidth(),
		displayMode->getHeight(),
		D3DFMT_LE_X8R8G8B8
	);

	return m_renderView;
}

IRenderView* RenderSystemXbox360::createRenderView(void* windowHandle, const RenderViewCreateDesc& desc)
{
	return 0;
}

VertexBuffer* RenderSystemXbox360::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	Ref< VertexBufferDx9 > vertexBuffer = gc_new< VertexBufferDx9 >(this, m_context, bufferSize, m_vertexDeclCache);
	if (!vertexBuffer->create(m_d3dDevice, vertexElements, dynamic))
		return 0;
	return vertexBuffer;
}

IndexBuffer* RenderSystemXbox360::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	Ref< IndexBufferDx9 > indexBuffer = gc_new< IndexBufferDx9 >(this, m_context, indexType, bufferSize);
	if (!indexBuffer->create(m_d3dDevice, dynamic))
		return 0;
	return indexBuffer;
}

ISimpleTexture* RenderSystemXbox360::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureDx9 > texture = gc_new< SimpleTextureDx9 >(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

ICubeTexture* RenderSystemXbox360::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTextureDx9 > texture = gc_new< CubeTextureDx9 >(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

IVolumeTexture* RenderSystemXbox360::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	Ref< VolumeTextureDx9 > texture = gc_new< VolumeTextureDx9 >(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

RenderTargetSet* RenderSystemXbox360::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	return 0;
}

ProgramResource* RenderSystemXbox360::compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate)
{
	HlslProgram program;
	if (!Hlsl().generate(shaderGraph, program))
		return 0;

	Ref< ProgramResourceDx9 > programResource = ProgramXbox360::compile(program, optimize, validate);
	if (!programResource)
		return 0;

	return programResource;
}

IProgram* RenderSystemXbox360::createProgram(const ProgramResource* programResource)
{
	T_ASSERT (m_parameterCache);

	Ref< const ProgramResourceDx9 > resource = dynamic_type_cast< const ProgramResourceDx9* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramXbox360 > program = gc_new< ProgramXbox360 >(this, m_context, m_parameterCache);
	if (!program->create(m_d3dDevice, resource))
		return 0;

	return program;
}

void RenderSystemXbox360::addUnmanaged(Unmanaged* unmanaged)
{
}

void RenderSystemXbox360::removeUnmanaged(Unmanaged* unmanaged)
{
}

	}
}
