#include "Render/Capture/RenderSystemCapture.h"
#include "Render/Capture/RenderViewCapture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderSystemCapture", RenderSystemCapture, IRenderSystem)

RenderSystemCapture::RenderSystemCapture(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

bool RenderSystemCapture::create(const RenderSystemCreateDesc& desc)
{
	return m_renderSystem->create(desc);
}

void RenderSystemCapture::destroy()
{
	m_renderSystem->destroy();
}

uint32_t RenderSystemCapture::getDisplayModeCount() const
{
	return m_renderSystem->getDisplayModeCount();
}

DisplayMode RenderSystemCapture::getDisplayMode(uint32_t index) const
{
	return m_renderSystem->getDisplayMode(index);
}

DisplayMode RenderSystemCapture::getCurrentDisplayMode() const
{
	return m_renderSystem->getCurrentDisplayMode();
}

bool RenderSystemCapture::handleMessages()
{
	return m_renderSystem->handleMessages();
}

Ref< IRenderView > RenderSystemCapture::createRenderView(const RenderViewCreateDefaultDesc& desc)
{
	Ref< IRenderView > renderView = m_renderSystem->createRenderView(desc);
	if (!renderView)
		return 0;

	return new RenderViewCapture(m_renderSystem, renderView);
}

Ref< IRenderView > RenderSystemCapture::createRenderView(const RenderViewCreateEmbeddedDesc& desc)
{
	Ref< IRenderView > renderView = m_renderSystem->createRenderView(desc);
	if (!renderView)
		return 0;

	return new RenderViewCapture(m_renderSystem, renderView);
}

Ref< VertexBuffer > RenderSystemCapture::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	return m_renderSystem->createVertexBuffer(vertexElements, bufferSize, dynamic);
}

Ref< IndexBuffer > RenderSystemCapture::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	return m_renderSystem->createIndexBuffer(indexType, bufferSize, dynamic);
}

Ref< ISimpleTexture > RenderSystemCapture::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	return m_renderSystem->createSimpleTexture(desc);
}

Ref< ICubeTexture > RenderSystemCapture::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	return m_renderSystem->createCubeTexture(desc);
}

Ref< IVolumeTexture > RenderSystemCapture::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	return m_renderSystem->createVolumeTexture(desc);
}

Ref< RenderTargetSet > RenderSystemCapture::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	return m_renderSystem->createRenderTargetSet(desc);
}

Ref< IProgram > RenderSystemCapture::createProgram(const ProgramResource* programResource)
{
	return m_renderSystem->createProgram(programResource);
}

Ref< IProgramCompiler > RenderSystemCapture::createProgramCompiler() const
{
	return m_renderSystem->createProgramCompiler();
}

	}
}
