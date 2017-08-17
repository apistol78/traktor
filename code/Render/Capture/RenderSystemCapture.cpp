/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/VertexElement.h"
#include "Render/Capture/CubeTextureCapture.h"
#include "Render/Capture/Error.h"
#include "Render/Capture/IndexBufferCapture.h"
#include "Render/Capture/ProgramCapture.h"
#include "Render/Capture/ProgramCompilerCapture.h"
#include "Render/Capture/ProgramResourceCapture.h"
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
	T_CAPTURE_ASSERT (bufferSize > 0, L"Invalid vertex buffer size.");

	uint32_t vertexSize = getVertexSize(vertexElements);
	T_CAPTURE_ASSERT (bufferSize % vertexSize == 0, L"Invalid vertex buffer size, is not aligned with size of vertex.");

	Ref< VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(vertexElements, bufferSize, dynamic);
	if (!vertexBuffer)
		return 0;

	return new VertexBufferCapture(vertexBuffer, bufferSize, vertexSize);
}

Ref< IndexBuffer > RenderSystemCapture::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	T_CAPTURE_ASSERT (bufferSize > 0, L"Invalid index buffer size.");

	Ref< IndexBuffer > indexBuffer = m_renderSystem->createIndexBuffer(indexType, bufferSize, dynamic);
	if (!indexBuffer)
		return 0;

	return new IndexBufferCapture(indexBuffer, indexType, bufferSize);
}

Ref< ISimpleTexture > RenderSystemCapture::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	T_CAPTURE_ASSERT (desc.width > 0, L"Invalid texture width.");
	T_CAPTURE_ASSERT (desc.height > 0, L"Invalid texture height.");
	T_CAPTURE_ASSERT (desc.mipCount >= 1, L"Invalid number of mips.");
	T_CAPTURE_ASSERT (desc.mipCount < 16, L"Too many mips.");

	if (desc.immutable)
	{
		for (int32_t i = 0; i < desc.mipCount; ++i)
		{
			T_CAPTURE_ASSERT (desc.initialData[i].data, L"No initial data of immutable texture.");
		}
	}

	Ref< ISimpleTexture > texture = m_renderSystem->createSimpleTexture(desc);
	if (!texture)
		return 0;

	return new SimpleTextureCapture(texture);
}

Ref< ICubeTexture > RenderSystemCapture::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	T_CAPTURE_ASSERT (desc.side > 0, L"Invalid cube texture size.");
	T_CAPTURE_ASSERT (desc.mipCount >= 1, L"Invalid number of mips.");
	T_CAPTURE_ASSERT (desc.mipCount < 16, L"Too many mips.");

	if (desc.immutable)
	{
		for (int32_t i = 0; i < desc.mipCount * 6; ++i)
		{
			T_CAPTURE_ASSERT (desc.initialData[i].data, L"No initial data of immutable texture.");
		}
	}

	Ref< ICubeTexture > texture = m_renderSystem->createCubeTexture(desc);
	if (!texture)
		return 0;

	return new CubeTextureCapture(texture);
}

Ref< IVolumeTexture > RenderSystemCapture::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	T_CAPTURE_ASSERT (desc.width > 0, L"Invalid volume texture width.");
	T_CAPTURE_ASSERT (desc.height > 0, L"Invalid volume texture height.");
	T_CAPTURE_ASSERT (desc.depth > 0, L"Invalid volume texture depth.");
	T_CAPTURE_ASSERT (desc.mipCount >= 1, L"Invalid number of mips.");
	T_CAPTURE_ASSERT (desc.mipCount < 16, L"Too many mips.");

	if (desc.immutable)
	{
		for (int32_t i = 0; i < desc.mipCount; ++i)
		{
			T_CAPTURE_ASSERT (desc.initialData[i].data, L"No initial data of immutable texture.");
		}
	}

	Ref< IVolumeTexture > texture = m_renderSystem->createVolumeTexture(desc);
	if (!texture)
		return 0;

	return new VolumeTextureCapture(texture);
}

Ref< RenderTargetSet > RenderSystemCapture::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	T_CAPTURE_ASSERT (desc.count >= 0, L"Negative number of targets.");
	T_CAPTURE_ASSERT (desc.count <= 4, L"Too many targets.");
	T_CAPTURE_ASSERT (desc.width > 0, L"Invalid size.");
	T_CAPTURE_ASSERT (desc.height > 0, L"Invalid size.");
	T_CAPTURE_ASSERT (desc.multiSample >= 0, L"Invalid multisample count.");

	Ref< RenderTargetSet > renderTargetSet = m_renderSystem->createRenderTargetSet(desc);
	if (!renderTargetSet)
		return 0;

	return new RenderTargetSetCapture(renderTargetSet);
}

Ref< IProgram > RenderSystemCapture::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	T_CAPTURE_ASSERT (programResource, L"No program resource.");

	if (!programResource)
		return 0;

	const ProgramResourceCapture* resource = dynamic_type_cast< const ProgramResourceCapture* >(programResource);
	T_CAPTURE_ASSERT (resource, L"Incorrect program resource type.");

	if (!resource)
		return 0;

	T_CAPTURE_ASSERT (resource->m_embedded, L"Invalid wrapped resource.");

	Ref< IProgram > program = m_renderSystem->createProgram(resource->m_embedded, tag);
	if (!program)
		return 0;

	Ref< ProgramCapture > programCapture = new ProgramCapture(program, tag);

	for (RefArray< Uniform >::const_iterator i = resource->m_uniforms.begin(); i != resource->m_uniforms.end(); ++i)
	{
		handle_t handle = getParameterHandle(i->getParameterName());
		programCapture->m_shadow[handle].uniform = *i;
		programCapture->m_shadow[handle].undefined = (i->getParameterType() <= PtMatrix) ? true : false;	// Textures are allowed to be unset, should sample black.
	}

	for (RefArray< IndexedUniform >::const_iterator i = resource->m_indexedUniforms.begin(); i != resource->m_indexedUniforms.end(); ++i)
	{
		handle_t handle = getParameterHandle(i->getParameterName());
		programCapture->m_shadow[handle].indexedUniform = *i;
		programCapture->m_shadow[handle].undefined = (i->getParameterType() <= PtMatrix) ? true : false;	// Textures are allowed to be unset, should sample black.
	}

	return programCapture;
}

Ref< IProgramCompiler > RenderSystemCapture::createProgramCompiler() const
{
	Ref< IProgramCompiler > compiler = m_renderSystem->createProgramCompiler();
	if (compiler)
		return new ProgramCompilerCapture(compiler);
	else
		return 0;
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
