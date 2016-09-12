#include "Render/VertexElement.h"
#include "Render/Capture/ProgramCapture.h"
#include "Render/Capture/RenderSystemCapture.h"
#include "Render/Capture/RenderViewCapture.h"
#include "Render/Capture/VertexBufferCapture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemCapture", 0, RenderSystemCapture, IRenderSystem)

bool RenderSystemCapture::create(const RenderSystemDesc& desc)
{
	if ((m_renderSystem = desc.capture) == 0)
		return false;

	return m_renderSystem->create(desc);
}

void RenderSystemCapture::destroy()
{
	m_renderSystem->destroy();
}

bool RenderSystemCapture::reset(const RenderSystemDesc& desc)
{
	return m_renderSystem->reset(desc);
}

void RenderSystemCapture::getInformation(RenderSystemInformation& outInfo) const
{
	m_renderSystem->getInformation(outInfo);
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

float RenderSystemCapture::getDisplayAspectRatio() const
{
	return m_renderSystem->getDisplayAspectRatio();
}

Ref< IRenderView > RenderSystemCapture::createRenderView(const RenderViewDefaultDesc& desc)
{
	Ref< IRenderView > renderView = m_renderSystem->createRenderView(desc);
	if (!renderView)
		return 0;

	return new RenderViewCapture(m_renderSystem, renderView);
}

Ref< IRenderView > RenderSystemCapture::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	Ref< IRenderView > renderView = m_renderSystem->createRenderView(desc);
	if (!renderView)
		return 0;

	return new RenderViewCapture(m_renderSystem, renderView);
}

Ref< VertexBuffer > RenderSystemCapture::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	Ref< VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(vertexElements, bufferSize, dynamic);
	if (!vertexBuffer)
		return 0;

	uint32_t vertexSize = getVertexSize(vertexElements);
	return new VertexBufferCapture(vertexBuffer, bufferSize, vertexSize);
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

Ref< IProgram > RenderSystemCapture::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	Ref< IProgram > program = m_renderSystem->createProgram(programResource, tag);
	return program ? new ProgramCapture(program, tag) : 0;
}

Ref< IProgramCompiler > RenderSystemCapture::createProgramCompiler() const
{
	return m_renderSystem->createProgramCompiler();
}

Ref< ITimeQuery > RenderSystemCapture::createTimeQuery() const
{
	return m_renderSystem->createTimeQuery();
}

void RenderSystemCapture::purge()
{
	m_renderSystem->purge();
}

void RenderSystemCapture::getStatistics(RenderSystemStatistics& outStatistics) const
{
	m_renderSystem->getStatistics(outStatistics);
}

	}
}
