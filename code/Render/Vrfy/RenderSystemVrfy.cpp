#if defined(_WIN32)
#	include <renderdoc_app.h>
#endif
#include "Core/Library/Library.h"
#include "Render/StructElement.h"
#include "Render/VertexElement.h"
#include "Render/Vrfy/CubeTextureVrfy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/IndexBufferVrfy.h"
#include "Render/Vrfy/ProgramVrfy.h"
#include "Render/Vrfy/ProgramResourceVrfy.h"
#include "Render/Vrfy/RenderSystemVrfy.h"
#include "Render/Vrfy/RenderTargetSetVrfy.h"
#include "Render/Vrfy/RenderViewVrfy.h"
#include "Render/Vrfy/SimpleTextureVrfy.h"
#include "Render/Vrfy/StructBufferVrfy.h"
#include "Render/Vrfy/VertexBufferVrfy.h"
#include "Render/Vrfy/VolumeTextureVrfy.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemVrfy", 0, RenderSystemVrfy, IRenderSystem)

bool RenderSystemVrfy::create(const RenderSystemDesc& desc)
{
	if ((m_renderSystem = desc.capture) == nullptr)
		return false;

#if defined(_WIN32) && !defined(_DEBUG)
	// Try to load RenderDoc capture.
	m_libRenderDoc = new Library();
	if (m_libRenderDoc->open(L"c:\\Program Files\\RenderDoc\\renderdoc.dll"))
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)m_libRenderDoc->find(L"RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_1, (void **)&m_apiRenderDoc);
		if (ret != 1)
			m_apiRenderDoc = nullptr;
	}
	else
		m_libRenderDoc = nullptr;

	//if (m_apiRenderDoc)
	//	m_apiRenderDoc->MaskOverlayBits(eRENDERDOC_Overlay_None, 0);
#endif

	return m_renderSystem->create(desc);
}

void RenderSystemVrfy::destroy()
{
	m_renderSystem->destroy();
}

bool RenderSystemVrfy::reset(const RenderSystemDesc& desc)
{
	return m_renderSystem->reset(desc);
}

void RenderSystemVrfy::getInformation(RenderSystemInformation& outInfo) const
{
	m_renderSystem->getInformation(outInfo);
}

uint32_t RenderSystemVrfy::getDisplayModeCount() const
{
	return m_renderSystem->getDisplayModeCount();
}

DisplayMode RenderSystemVrfy::getDisplayMode(uint32_t index) const
{
	return m_renderSystem->getDisplayMode(index);
}

DisplayMode RenderSystemVrfy::getCurrentDisplayMode() const
{
	return m_renderSystem->getCurrentDisplayMode();
}

float RenderSystemVrfy::getDisplayAspectRatio() const
{
	return m_renderSystem->getDisplayAspectRatio();
}

Ref< IRenderView > RenderSystemVrfy::createRenderView(const RenderViewDefaultDesc& desc)
{
	Ref< IRenderView > renderView = m_renderSystem->createRenderView(desc);
	if (!renderView)
		return nullptr;

	return new RenderViewVrfy(desc, m_renderSystem, renderView);
}

Ref< IRenderView > RenderSystemVrfy::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	Ref< IRenderView > renderView = m_renderSystem->createRenderView(desc);
	if (!renderView)
		return nullptr;

	return new RenderViewVrfy(desc, m_renderSystem, renderView);
}

Ref< VertexBuffer > RenderSystemVrfy::createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	T_CAPTURE_ASSERT(bufferSize > 0, L"Invalid vertex buffer size.");

	uint32_t vertexSize = getVertexSize(vertexElements);
	T_CAPTURE_ASSERT(bufferSize % vertexSize == 0, L"Invalid vertex buffer size, is not aligned with size of vertex.");

	Ref< VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(vertexElements, bufferSize, dynamic);
	if (!vertexBuffer)
		return nullptr;

	return new VertexBufferVrfy(vertexBuffer, bufferSize, vertexSize);
}

Ref< IndexBuffer > RenderSystemVrfy::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	T_CAPTURE_ASSERT(bufferSize > 0, L"Invalid index buffer size.");

	Ref< IndexBuffer > indexBuffer = m_renderSystem->createIndexBuffer(indexType, bufferSize, dynamic);
	if (!indexBuffer)
		return nullptr;

	return new IndexBufferVrfy(indexBuffer, indexType, bufferSize);
}

Ref< StructBuffer > RenderSystemVrfy::createStructBuffer(const AlignedVector< StructElement >& structElements, uint32_t bufferSize, bool dynamic)
{
	T_CAPTURE_ASSERT(bufferSize > 0, L"Invalid structure buffer size.");

	uint32_t structSize = getStructSize(structElements);
	T_CAPTURE_ASSERT(bufferSize % structSize == 0, L"Invalid struct buffer size, is not aligned with size of struct.");

	Ref< StructBuffer > structBuffer = m_renderSystem->createStructBuffer(structElements, bufferSize, dynamic);
	if (!structBuffer)
		return nullptr;

	return new StructBufferVrfy(structBuffer, bufferSize, structSize);	
}

Ref< ISimpleTexture > RenderSystemVrfy::createSimpleTexture(const SimpleTextureCreateDesc& desc, const wchar_t* const tag)
{
	T_CAPTURE_ASSERT(desc.width > 0, L"Invalid texture width.");
	T_CAPTURE_ASSERT(desc.height > 0, L"Invalid texture height.");
	T_CAPTURE_ASSERT(desc.mipCount >= 1, L"Invalid number of mips.");
	T_CAPTURE_ASSERT(desc.mipCount < 16, L"Too many mips.");

	if (desc.immutable)
	{
		for (int32_t i = 0; i < desc.mipCount; ++i)
		{
			T_CAPTURE_ASSERT(desc.initialData[i].data, L"No initial data of immutable texture.");
		}
	}

	Ref< ISimpleTexture > texture = m_renderSystem->createSimpleTexture(desc, tag);
	if (!texture)
		return nullptr;

	return new SimpleTextureVrfy(texture);
}

Ref< ICubeTexture > RenderSystemVrfy::createCubeTexture(const CubeTextureCreateDesc& desc, const wchar_t* const tag)
{
	T_CAPTURE_ASSERT(desc.side > 0, L"Invalid cube texture size.");
	T_CAPTURE_ASSERT(desc.mipCount >= 1, L"Invalid number of mips.");
	T_CAPTURE_ASSERT(desc.mipCount < 16, L"Too many mips.");

	if (desc.immutable)
	{
		for (int32_t i = 0; i < desc.mipCount * 6; ++i)
		{
			T_CAPTURE_ASSERT(desc.initialData[i].data, L"No initial data of immutable texture.");
		}
	}

	Ref< ICubeTexture > texture = m_renderSystem->createCubeTexture(desc, tag);
	if (!texture)
		return nullptr;

	return new CubeTextureVrfy(texture);
}

Ref< IVolumeTexture > RenderSystemVrfy::createVolumeTexture(const VolumeTextureCreateDesc& desc, const wchar_t* const tag)
{
	T_CAPTURE_ASSERT(desc.width > 0, L"Invalid volume texture width.");
	T_CAPTURE_ASSERT(desc.height > 0, L"Invalid volume texture height.");
	T_CAPTURE_ASSERT(desc.depth > 0, L"Invalid volume texture depth.");
	T_CAPTURE_ASSERT(desc.mipCount >= 1, L"Invalid number of mips.");
	T_CAPTURE_ASSERT(desc.mipCount < 16, L"Too many mips.");

	if (desc.immutable)
	{
		for (int32_t i = 0; i < desc.mipCount; ++i)
		{
			T_CAPTURE_ASSERT(desc.initialData[i].data, L"No initial data of immutable texture.");
		}
	}

	Ref< IVolumeTexture > texture = m_renderSystem->createVolumeTexture(desc, tag);
	if (!texture)
		return nullptr;

	return new VolumeTextureVrfy(texture);
}

Ref< IRenderTargetSet > RenderSystemVrfy::createRenderTargetSet(const RenderTargetSetCreateDesc& desc, IRenderTargetSet* sharedDepthStencil, const wchar_t* const tag)
{
	T_CAPTURE_ASSERT(desc.count >= 0, L"Negative number of targets.");
	T_CAPTURE_ASSERT(desc.count <= 4, L"Too many targets.");
	T_CAPTURE_ASSERT(desc.width > 0, L"Invalid size.");
	T_CAPTURE_ASSERT(desc.height > 0, L"Invalid size.");
	T_CAPTURE_ASSERT(desc.multiSample >= 0, L"Invalid multisample count.");

	if (desc.createDepthStencil)
	{
		T_CAPTURE_ASSERT(!desc.usingPrimaryDepthStencil, L"Invalid values in create desc.");
		T_CAPTURE_ASSERT(sharedDepthStencil == nullptr, L"Invalid values in create desc.");
	}
	
	if (sharedDepthStencil)
	{
		T_CAPTURE_ASSERT(!desc.createDepthStencil, L"Invalid values in create desc.");
		T_CAPTURE_ASSERT(!desc.usingPrimaryDepthStencil, L"Invalid values in create desc.");
	}

	if (desc.usingPrimaryDepthStencil)
	{
		T_CAPTURE_ASSERT(!desc.createDepthStencil, L"Invalid values in create desc.");
		T_CAPTURE_ASSERT(!desc.usingDepthStencilAsTexture, L"Invalid values in create desc.");
		T_CAPTURE_ASSERT(sharedDepthStencil == nullptr, L"Invalid values in create desc.");
	}

	Ref< IRenderTargetSet > renderTargetSet;
	if (sharedDepthStencil)
	{
		Ref< RenderTargetSetVrfy > sharedDepthStencilVrfy = dynamic_type_cast< RenderTargetSetVrfy* >(sharedDepthStencil);
		T_CAPTURE_ASSERT(sharedDepthStencilVrfy, L"Not correct type of render target set.");

		if (!sharedDepthStencilVrfy)
			return nullptr;
		T_CAPTURE_ASSERT(sharedDepthStencilVrfy->getRenderTargetSet(), L"Using destroyed render target set.");

		T_CAPTURE_ASSERT(sharedDepthStencilVrfy->getWidth() == desc.width, L"Incompatible width.");
		T_CAPTURE_ASSERT(sharedDepthStencilVrfy->getHeight() == desc.height, L"Incompatible height.");
		T_CAPTURE_ASSERT(sharedDepthStencilVrfy->getMultiSample() == desc.multiSample, L"Incompatible multisample.");
		T_CAPTURE_ASSERT(sharedDepthStencilVrfy->haveDepthTexture(), L"Not depth/stencil in shared set.");

		renderTargetSet = m_renderSystem->createRenderTargetSet(desc, sharedDepthStencilVrfy->getRenderTargetSet(), tag);
	}
	else
		renderTargetSet = m_renderSystem->createRenderTargetSet(desc, nullptr, tag);
	
	if (!renderTargetSet)
		return nullptr;

	return new RenderTargetSetVrfy(desc, renderTargetSet);
}

Ref< IProgram > RenderSystemVrfy::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	T_CAPTURE_ASSERT(programResource, L"No program resource.");

	if (!programResource)
		return nullptr;

	Ref< IProgram > program;

	const ProgramResourceVrfy* resource = dynamic_type_cast< const ProgramResourceVrfy* >(programResource);
	if (resource != nullptr)
	{
		T_CAPTURE_ASSERT(resource->m_embedded, L"Invalid wrapped resource.");
		program = m_renderSystem->createProgram(resource->m_embedded, tag);
	}
	else
		program = m_renderSystem->createProgram(programResource, tag);

	if (!program)
		return nullptr;

	Ref< ProgramVrfy > programVrfy = new ProgramVrfy(program, tag);

	// for (RefArray< Uniform >::const_iterator i = resource->m_uniforms.begin(); i != resource->m_uniforms.end(); ++i)
	// {
	// 	handle_t handle = getParameterHandle(i->getParameterName());
	// 	programVrfy->m_shadow[handle].uniform = *i;
	// 	programVrfy->m_shadow[handle].undefined = (i->getParameterType() <= PtMatrix) ? true : false;	// Textures are allowed to be unset, should sample black.
	// }

	// for (RefArray< IndexedUniform >::const_iterator i = resource->m_indexedUniforms.begin(); i != resource->m_indexedUniforms.end(); ++i)
	// {
	// 	handle_t handle = getParameterHandle(i->getParameterName());
	// 	programVrfy->m_shadow[handle].indexedUniform = *i;
	// 	programVrfy->m_shadow[handle].undefined = (i->getParameterType() <= PtMatrix) ? true : false;	// Textures are allowed to be unset, should sample black.
	// }

	return programVrfy;
}

void RenderSystemVrfy::purge()
{
	m_renderSystem->purge();
}

void RenderSystemVrfy::getStatistics(RenderSystemStatistics& outStatistics) const
{
	m_renderSystem->getStatistics(outStatistics);
}

	}
}
