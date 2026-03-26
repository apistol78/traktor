/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if defined(_WIN32) || defined(__LINUX__)
#	include <GFSDK_Aftermath.h>
#	include <GFSDK_Aftermath_GpuCrashDump.h>
#	include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>
#endif
#if defined(_WIN32) || defined(__LINUX__)
#	include <renderdoc_app.h>
#endif
#include "Render/Vrfy/RenderSystemVrfy.h"

#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Library/Library.h"
#include "Core/Misc/String.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Render/VertexElement.h"
#include "Render/Vrfy/AccelerationStructureVrfy.h"
#include "Render/Vrfy/BufferVrfy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/ProgramResourceVrfy.h"
#include "Render/Vrfy/ProgramVrfy.h"
#include "Render/Vrfy/RenderPluginVrfy.h"
#include "Render/Vrfy/RenderTargetSetVrfy.h"
#include "Render/Vrfy/RenderViewVrfy.h"
#include "Render/Vrfy/ResourceTracker.h"
#include "Render/Vrfy/TextureVrfy.h"
#include "Render/Vrfy/VertexLayoutVrfy.h"

namespace traktor::render
{
namespace
{

#if defined(_WIN32) || defined(__LINUX__)

Semaphore s_aftermathLock;

void aftermathGpuCrashDumpCallback(const void* crashDump, const uint32_t crashDumpSize, void* userData)
{
	const std::wstring filename = OS::getInstance().getWritableFolderPath() + L"/Traktor/traktor_gpu_crash.nv-gpudmp";
	log::error << L"GPU crash detected! Writing GPU crash dump \"" << filename << L"\"..." << Endl;

	Ref< IStream > f = FileSystem::getInstance().open(filename, File::FmWrite);
	if (f)
	{
		f->write(crashDump, crashDumpSize);
		f->close();
		f = nullptr;
	}
	else
		log::error << L"Unable to create GPU crash dump file!" << Endl;
}

void aftermathShaderDebugInfo(const void* shaderDebugInfo, const uint32_t shaderDebugInfoSize, void* userData)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_aftermathLock);

	GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier = {};
	GFSDK_Aftermath_GetShaderDebugInfoIdentifier(GFSDK_Aftermath_Version_API, shaderDebugInfo, shaderDebugInfoSize, &identifier);

	const std::wstring filename = OS::getInstance().getWritableFolderPath() + L"/Traktor/" + str(L"%016x%016x", identifier.id[0], identifier.id[1]);
	Ref< IStream > f = FileSystem::getInstance().open(filename, File::FmWrite);
	if (f)
	{
		f->write(shaderDebugInfo, shaderDebugInfoSize);
		f->close();
		f = nullptr;
	}
	else
		log::error << L"Unable to create shader debug information file!" << Endl;
}

#endif

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemVrfy", 0, RenderSystemVrfy, IRenderSystem)

RenderSystemVrfy::RenderSystemVrfy(bool useRenderDoc, bool useAftermath)
	: m_useRenderDoc(useRenderDoc)
	, m_useAftermath(useAftermath)
{
}

bool RenderSystemVrfy::create(const RenderSystemDesc& desc)
{
	if ((m_renderSystem = desc.capture) == nullptr)
		return false;

#if defined(_WIN32) || defined(__LINUX__)
	if (m_useAftermath)
	{
		// Load NVidia aftermath.
		const GFSDK_Aftermath_Result result = GFSDK_Aftermath_EnableGpuCrashDumps(
			GFSDK_Aftermath_Version_API,
			GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan,
			GFSDK_Aftermath_GpuCrashDumpFeatureFlags_Default,
			aftermathGpuCrashDumpCallback, // Register callback for GPU crash dumps.
			aftermathShaderDebugInfo,	   // Register callback for shader debug information.
			nullptr,					   // Register callback for GPU crash dump description.
			nullptr,					   // Register callback for marker resolution (R495 or later NVIDIA graphics driver).
			nullptr);
		if (result == GFSDK_Aftermath_Result_Success)
		{
			log::info << L"NV Aftermath integration initialized." << Endl;

			// We cannot have RD enabled if we are using the aftermath SDK; not compatible.
			m_useRenderDoc = false;
		}
		else
		{
			log::warning << L"NV Aftermath integration failed." << Endl;
			m_useAftermath = false;
		}
	}
#endif

#if defined(_WIN32) || defined(__LINUX__)
	// Try to load RenderDoc capture.
	if (m_useRenderDoc)
	{
#	if defined(_WIN32)
		const std::wstring renderDocDLL = Path(L"renderdoc.dll").getPathNameOS();
#	else
		const std::wstring renderDocDLL = Path(L"renderdoc").getPathNameOS();
#	endif

		m_libRenderDoc = new Library();
		if (m_libRenderDoc->open(renderDocDLL.c_str()))
		{
			pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)m_libRenderDoc->find(L"RENDERDOC_GetAPI");
			const int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_7_0, (void**)&m_apiRenderDoc);
			if (ret != 1)
				m_apiRenderDoc = nullptr;
		}
		else
		{
			m_libRenderDoc = nullptr;
			m_useRenderDoc = false;
		}

		if (m_apiRenderDoc)
		{
			((RENDERDOC_API_1_7_0*)m_apiRenderDoc)->SetCaptureTitle("Traktor");
			((RENDERDOC_API_1_7_0*)m_apiRenderDoc)->MaskOverlayBits(eRENDERDOC_Overlay_All, eRENDERDOC_Overlay_Default);
			log::info << L"RenderDoc integration initialized." << Endl;
		}
	}
#endif

	RenderSystemDesc mutableDesc = desc;
	mutableDesc.aftermath = m_useAftermath;
	if (!m_renderSystem->create(mutableDesc))
		return false;

	m_resourceTracker = new ResourceTracker();
	return true;
}

void RenderSystemVrfy::destroy()
{
	m_resourceTracker->alive(type_of< Object >(), false);
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

bool RenderSystemVrfy::supportRayTracing() const
{
	return m_renderSystem->supportRayTracing();
}

uint32_t RenderSystemVrfy::getDisplayCount() const
{
	return m_renderSystem->getDisplayCount();
}

uint32_t RenderSystemVrfy::getDisplayModeCount(uint32_t display) const
{
	return m_renderSystem->getDisplayModeCount(display);
}

DisplayMode RenderSystemVrfy::getDisplayMode(uint32_t display, uint32_t index) const
{
	return m_renderSystem->getDisplayMode(display, index);
}

DisplayMode RenderSystemVrfy::getCurrentDisplayMode(uint32_t display) const
{
	return m_renderSystem->getCurrentDisplayMode(display);
}

float RenderSystemVrfy::getDisplayAspectRatio(uint32_t display) const
{
	return m_renderSystem->getDisplayAspectRatio(display);
}

Ref< IRenderView > RenderSystemVrfy::createRenderView(const RenderViewDefaultDesc& desc)
{
	T_CAPTURE_TRACE(L"createRenderView");

	Ref< IRenderView > renderView = m_renderSystem->createRenderView(desc);
	if (!renderView)
		return nullptr;

	return new RenderViewVrfy(desc, m_renderSystem, renderView);
}

Ref< IRenderView > RenderSystemVrfy::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	T_CAPTURE_TRACE(L"createRenderView");

	Ref< IRenderView > renderView = m_renderSystem->createRenderView(desc);
	if (!renderView)
		return nullptr;

#if defined(_WIN32)
	if (m_apiRenderDoc)
		((RENDERDOC_API_1_7_0*)m_apiRenderDoc)->SetActiveWindow(m_renderSystem->getInternalHandle(), desc.syswin.hWnd);
#elif defined(__LINUX__)
	if (m_apiRenderDoc)
		((RENDERDOC_API_1_7_0*)m_apiRenderDoc)->SetActiveWindow(m_renderSystem->getInternalHandle(), (RENDERDOC_WindowHandle)desc.syswin.window);
#endif

	return new RenderViewVrfy(desc, m_renderSystem, renderView);
}

Ref< Buffer > RenderSystemVrfy::createBuffer(uint32_t usage, uint32_t bufferSize, bool dynamic, const wchar_t* const tag)
{
	T_CAPTURE_TRACE(L"createBuffer");
	T_CAPTURE_ASSERT(usage != 0, L"Invalid usage.");
	T_CAPTURE_ASSERT(bufferSize > 0, L"Invalid buffer size.");

	Ref< Buffer > buffer = m_renderSystem->createBuffer(usage, bufferSize, dynamic, tag);
	if (!buffer)
		return nullptr;

	return new BufferVrfy(m_resourceTracker, buffer, bufferSize, tag);
}

Ref< const IVertexLayout > RenderSystemVrfy::createVertexLayout(const AlignedVector< VertexElement >& vertexElements)
{
	T_CAPTURE_TRACE(L"createBuffer");
	T_CAPTURE_ASSERT(!vertexElements.empty(), L"Invalid vertex layout.");

	Ref< const IVertexLayout > vertexLayout = m_renderSystem->createVertexLayout(vertexElements);
	if (!vertexLayout)
		return nullptr;

	return new VertexLayoutVrfy(vertexLayout, getVertexSize(vertexElements));
}

Ref< ITexture > RenderSystemVrfy::createSimpleTexture(const SimpleTextureCreateDesc& desc, const wchar_t* const tag)
{
	T_CAPTURE_TRACE(L"createSimpleTexture");
	T_CAPTURE_ASSERT(desc.width > 0, L"Invalid texture width.");
	T_CAPTURE_ASSERT(desc.height > 0, L"Invalid texture height.");
	T_CAPTURE_ASSERT(desc.mipCount >= 1, L"Invalid number of mips.");
	T_CAPTURE_ASSERT(desc.mipCount < 16, L"Too many mips.");

	if (desc.immutable)
		for (int32_t i = 0; i < desc.mipCount; ++i)
			T_CAPTURE_ASSERT(desc.initialData[i].data, L"No initial data of immutable texture.");

	Ref< ITexture > texture = m_renderSystem->createSimpleTexture(desc, tag);
	if (!texture)
		return nullptr;

	return new TextureVrfy(m_resourceTracker, texture, desc.shaderStorage);
}

Ref< ITexture > RenderSystemVrfy::createCubeTexture(const CubeTextureCreateDesc& desc, const wchar_t* const tag)
{
	T_CAPTURE_TRACE(L"createCubeTexture");
	T_CAPTURE_ASSERT(desc.side > 0, L"Invalid cube texture size.");
	T_CAPTURE_ASSERT(desc.mipCount >= 1, L"Invalid number of mips.");
	T_CAPTURE_ASSERT(desc.mipCount < 16, L"Too many mips.");

	if (desc.immutable)
		for (int32_t i = 0; i < desc.mipCount * 6; ++i)
			T_CAPTURE_ASSERT(desc.initialData[i].data, L"No initial data of immutable texture.");

	Ref< ITexture > texture = m_renderSystem->createCubeTexture(desc, tag);
	if (!texture)
		return nullptr;

	return new TextureVrfy(m_resourceTracker, texture, desc.shaderStorage);
}

Ref< ITexture > RenderSystemVrfy::createVolumeTexture(const VolumeTextureCreateDesc& desc, const wchar_t* const tag)
{
	T_CAPTURE_TRACE(L"createVolumeTexture");
	T_CAPTURE_ASSERT(desc.width > 0, L"Invalid volume texture width.");
	T_CAPTURE_ASSERT(desc.height > 0, L"Invalid volume texture height.");
	T_CAPTURE_ASSERT(desc.depth > 0, L"Invalid volume texture depth.");
	T_CAPTURE_ASSERT(desc.mipCount >= 1, L"Invalid number of mips.");
	T_CAPTURE_ASSERT(desc.mipCount < 16, L"Too many mips.");

	if (desc.immutable)
		for (int32_t i = 0; i < desc.mipCount; ++i)
			T_CAPTURE_ASSERT(desc.initialData[i].data, L"No initial data of immutable texture.");

	Ref< ITexture > texture = m_renderSystem->createVolumeTexture(desc, tag);
	if (!texture)
		return nullptr;

	return new TextureVrfy(m_resourceTracker, texture, desc.shaderStorage);
}

Ref< IRenderTargetSet > RenderSystemVrfy::createRenderTargetSet(const RenderTargetSetCreateDesc& desc, IRenderTargetSet* sharedDepthStencil, const wchar_t* const tag)
{
	T_CAPTURE_TRACE(L"createRenderTargetSet");
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
		T_CAPTURE_ASSERT(!desc.usingDepthStencilAsStorage, L"Invalid values in create desc.");
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

	return new RenderTargetSetVrfy(m_resourceTracker, desc, renderTargetSet);
}

Ref< IAccelerationStructure > RenderSystemVrfy::createTopLevelAccelerationStructure(uint32_t numInstances)
{
	T_CAPTURE_TRACE(L"createTopLevelAccelerationStructure");

	Ref< IAccelerationStructure > as = m_renderSystem->createTopLevelAccelerationStructure(numInstances);
	if (!as)
		return nullptr;

	return new AccelerationStructureVrfy(as);
}

Ref< IAccelerationStructure > RenderSystemVrfy::createAccelerationStructure(const Buffer* vertexBuffer, const IVertexLayout* vertexLayout, const Buffer* indexBuffer, IndexType indexType, const AlignedVector< RaytracingPrimitives >& primitives, bool dynamic)
{
	T_CAPTURE_TRACE(L"createAccelerationStructure");

	const BufferVrfy* vb = mandatory_non_null_type_cast< const BufferVrfy* >(vertexBuffer);
	const BufferVrfy* ib = mandatory_non_null_type_cast< const BufferVrfy* >(indexBuffer);
	const VertexLayoutVrfy* vl = mandatory_non_null_type_cast< const VertexLayoutVrfy* >(vertexLayout);

	Ref< IAccelerationStructure > as = m_renderSystem->createAccelerationStructure(vb->getWrappedBuffer(), vl->getWrappedVertexLayout(), ib->getWrappedBuffer(), indexType, primitives, dynamic);
	if (!as)
		return nullptr;

	return new AccelerationStructureVrfy(as);
}

Ref< IProgram > RenderSystemVrfy::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	T_CAPTURE_TRACE(L"createProgram");
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

	return new ProgramVrfy(m_resourceTracker, resource, program, tag);
}

void RenderSystemVrfy::purge()
{
	m_renderSystem->purge();
}

void RenderSystemVrfy::getStatistics(RenderSystemStatistics& outStatistics) const
{
	m_renderSystem->getStatistics(outStatistics);

	outStatistics.buffers = m_resourceTracker->count(type_of< BufferVrfy >());
	outStatistics.renderTargetSets = m_resourceTracker->count(type_of< RenderTargetSetVrfy >());
	outStatistics.programs = m_resourceTracker->count(type_of< ProgramVrfy >());

	static int32_t dump = 0;
	if (dump == 1)
		m_resourceTracker->snapshot();
	else if (dump == 2)
		m_resourceTracker->alive(type_of< BufferVrfy >(), true);
	dump = 0;
}

void* RenderSystemVrfy::getInternalHandle() const
{
	return m_renderSystem->getInternalHandle();
}

Ref< IRenderPlugin > RenderSystemVrfy::createPlugin(const TypeInfo& pluginType)
{
	Ref< IRenderPlugin > plugin = m_renderSystem->createPlugin(pluginType);
	if (plugin)
		return new RenderPluginVrfy(plugin);
	else
		return nullptr;
}

}
