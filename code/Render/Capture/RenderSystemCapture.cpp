#include "Render/VertexElement.h"
#include "Render/Capture/CubeTextureCapture.h"
#include "Render/Capture/IndexBufferCapture.h"
#include "Render/Capture/ProgramCapture.h"
#include "Render/Capture/RenderSystemCapture.h"
#include "Render/Capture/RenderTargetSetCapture.h"
#include "Render/Capture/RenderViewCapture.h"
#include "Render/Capture/SimpleTextureCapture.h"
#include "Render/Capture/VertexBufferCapture.h"
#include "Render/Capture/VolumeTextureCapture.h"

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
	T_FATAL_ASSERT_M (bufferSize > 0, L"Render error: Invalid buffer size.");

	uint32_t vertexSize = getVertexSize(vertexElements);
	T_FATAL_ASSERT_M (bufferSize % vertexSize == 0, L"Render error: Invalid buffer size, is not aligned with size of vertex.");

	Ref< VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(vertexElements, bufferSize, dynamic);
	if (!vertexBuffer)
		return 0;

	return new VertexBufferCapture(vertexBuffer, bufferSize, vertexSize);
}

Ref< IndexBuffer > RenderSystemCapture::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	T_FATAL_ASSERT_M (bufferSize > 0, L"Render error: Invalid buffer size.");

	Ref< IndexBuffer > indexBuffer = m_renderSystem->createIndexBuffer(indexType, bufferSize, dynamic);
	if (!indexBuffer)
		return 0;

	return new IndexBufferCapture(indexBuffer, indexType, bufferSize);
}

Ref< ISimpleTexture > RenderSystemCapture::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	T_FATAL_ASSERT_M (desc.width > 0, L"Render error: Invalid size.");
	T_FATAL_ASSERT_M (desc.height > 0, L"Render error: Invalid size.");
	T_FATAL_ASSERT_M (desc.mipCount >= 1, L"Render error: Invalid number of mips.");
	T_FATAL_ASSERT_M (desc.mipCount < 16, L"Render error: Too many mips.");

	if (desc.immutable)
	{
		for (int32_t i = 0; i < desc.mipCount; ++i)
		{
			T_FATAL_ASSERT_M (desc.initialData[i].data, L"Render error: No initial data of immutable texture.");
		}
	}

	Ref< ISimpleTexture > texture = m_renderSystem->createSimpleTexture(desc);
	if (!texture)
		return 0;

	return new SimpleTextureCapture(texture);
}

Ref< ICubeTexture > RenderSystemCapture::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	T_FATAL_ASSERT_M (desc.side > 0, L"Render error: Invalid size.");
	T_FATAL_ASSERT_M (desc.mipCount >= 1, L"Render error: Invalid number of mips.");
	T_FATAL_ASSERT_M (desc.mipCount < 16, L"Render error: Too many mips.");

	if (desc.immutable)
	{
		for (int32_t i = 0; i < desc.mipCount * 6; ++i)
		{
			T_FATAL_ASSERT_M (desc.initialData[i].data, L"Render error: No initial data of immutable texture.");
		}
	}

	Ref< ICubeTexture > texture = m_renderSystem->createCubeTexture(desc);
	if (!texture)
		return 0;

	return new CubeTextureCapture(texture);
}

Ref< IVolumeTexture > RenderSystemCapture::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	T_FATAL_ASSERT_M (desc.width > 0, L"Render error: Invalid size.");
	T_FATAL_ASSERT_M (desc.height > 0, L"Render error: Invalid size.");
	T_FATAL_ASSERT_M (desc.depth > 0, L"Render error: Invalid size.");
	T_FATAL_ASSERT_M (desc.mipCount >= 1, L"Render error: Invalid number of mips.");
	T_FATAL_ASSERT_M (desc.mipCount < 16, L"Render error: Too many mips.");

	if (desc.immutable)
	{
		for (int32_t i = 0; i < desc.mipCount; ++i)
		{
			T_FATAL_ASSERT_M (desc.initialData[i].data, L"Render error: No initial data of immutable texture.");
		}
	}

	Ref< IVolumeTexture > texture = m_renderSystem->createVolumeTexture(desc);
	if (!texture)
		return 0;

	return new VolumeTextureCapture(texture);
}

Ref< RenderTargetSet > RenderSystemCapture::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	T_FATAL_ASSERT_M (desc.count > 0, L"Render error: Must have atleast one target.");
	T_FATAL_ASSERT_M (desc.count < 4, L"Render error: Too many targets.");
	T_FATAL_ASSERT_M (desc.width < 0, L"Render error: Invalid size.");
	T_FATAL_ASSERT_M (desc.height < 0, L"Render error: Invalid size.");
	T_FATAL_ASSERT_M (desc.multiSample >= 0, L"Render error: Invalid multisample count.");

	Ref< RenderTargetSet > renderTargetSet = m_renderSystem->createRenderTargetSet(desc);
	if (!renderTargetSet)
		return 0;

	return new RenderTargetSetCapture(renderTargetSet);
}

Ref< IProgram > RenderSystemCapture::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	T_FATAL_ASSERT_M (programResource, L"Render error: No program resource.");
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
