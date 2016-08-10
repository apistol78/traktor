#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/CubeTextureVk.h"
#include "Render/Vulkan/IndexBufferDynamicVk.h"
#include "Render/Vulkan/IndexBufferStaticVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/ProgramCompilerVk.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/RenderSystemVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/RenderViewVk.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/TimeQueryVk.h"
#include "Render/Vulkan/VertexBufferDynamicVk.h"
#include "Render/Vulkan/VertexBufferStaticVk.h"
#include "Render/Vulkan/VolumeTextureVk.h"
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/ApiLoader.h"
#	include "Render/Vulkan/Win32/Window.h"
#endif

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemVk", 0, RenderSystemVk, IRenderSystem)

RenderSystemVk::RenderSystemVk()
:	m_vkInstance(0)
{
}

bool RenderSystemVk::create(const RenderSystemDesc& desc)
{
#if defined(_WIN32)

	if (!initializeVulkanApi())
		return false;

	// Create render window; used by default render view and also by shared context.
	m_window = new Window();
	if (!m_window->create())
	{
		log::error << L"Failed to create render window." << Endl;
		return false;
	}

#endif

	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, 0);
	if (layerCount == 0)
	{
		log::error << L"Failed to create Vulkan instance layer properties." << Endl;
		return false;
	}
 
	VkLayerProperties* layersAvailable = new VkLayerProperties[layerCount];
	vkEnumerateInstanceLayerProperties(&layerCount, layersAvailable);

	const char* validationLayerNames[] = { "VK_LAYER_LUNARG_standard_validation" };
	bool foundValidation = false;
	for (uint32_t i = 0; i < layerCount; ++i)
	{
	   if (strcmp(layersAvailable[i].layerName, validationLayerNames[0]) == 0)
	   {
			foundValidation = true;
			break;
	   }
	}
	if (!foundValidation)
		log::warning << L"No Vulkan validation layer found." << Endl;

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = 0;
	applicationInfo.pApplicationName = "Traktor";
	applicationInfo.pEngineName = "Traktor";
	applicationInfo.engineVersion = 1;
	applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &applicationInfo;
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = 0;
	instanceInfo.enabledExtensionCount = 0;
	instanceInfo.ppEnabledExtensionNames = 0;

	if (foundValidation)
	{
		instanceInfo.enabledLayerCount = 1;
		instanceInfo.ppEnabledLayerNames = validationLayerNames;
	}
	
	if (vkCreateInstance(&instanceInfo, 0, &m_vkInstance) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan instance." << Endl;
		return false;
	}

	return true;
}

void RenderSystemVk::destroy()
{
#if defined(_WIN32)

	finalizeVulkanApi();

#endif
}

bool RenderSystemVk::reset(const RenderSystemDesc& desc)
{
	// \todo Update mipmap bias and maximum anisotropy.
	return true;
}

void RenderSystemVk::getInformation(RenderSystemInformation& outInfo) const
{
	outInfo.dedicatedMemoryTotal = 0;
	outInfo.sharedMemoryTotal = 0;
	outInfo.dedicatedMemoryAvailable = 0;
	outInfo.sharedMemoryAvailable = 0;
}

uint32_t RenderSystemVk::getDisplayModeCount() const
{
	return 0;
}

DisplayMode RenderSystemVk::getDisplayMode(uint32_t index) const
{
	DisplayMode dm;
	return dm;
}

DisplayMode RenderSystemVk::getCurrentDisplayMode() const
{
	DisplayMode cdm;
	return cdm;
}

float RenderSystemVk::getDisplayAspectRatio() const
{
	return 1.0f;
}

Ref< IRenderView > RenderSystemVk::createRenderView(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32)
	Ref< RenderViewVk > renderView = new RenderViewVk(m_window);
#else
	Ref< RenderViewVk > renderView = new RenderViewVk();
#endif

	if (!renderView->reset(desc))
		return 0;

	return renderView;
}

Ref< IRenderView > RenderSystemVk::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	return 0;
}

Ref< VertexBuffer > RenderSystemVk::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	if (!dynamic)
		return VertexBufferStaticVk::create(bufferSize, vertexElements);
	else
		return VertexBufferDynamicVk::create(bufferSize, vertexElements);
}

Ref< IndexBuffer > RenderSystemVk::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	if (!dynamic)
		return IndexBufferStaticVk::create(indexType, bufferSize);
	else
		return IndexBufferDynamicVk::create(indexType, bufferSize);
}

Ref< ISimpleTexture > RenderSystemVk::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureVk > texture = new SimpleTextureVk();
	if (texture->create(desc))
		return texture;
	else
		return 0;
}

Ref< ICubeTexture > RenderSystemVk::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTextureVk > texture = new CubeTextureVk();
	if (texture->create(desc))
		return texture;
	else
		return 0;
}

Ref< IVolumeTexture > RenderSystemVk::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	Ref< VolumeTextureVk > texture = new VolumeTextureVk();
	if (texture->create(desc))
		return texture;
	else
		return 0;
}

Ref< RenderTargetSet > RenderSystemVk::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetVk > renderTargetSet = new RenderTargetSetVk();
	if (renderTargetSet->create(desc))
		return renderTargetSet;
	else
		return 0;
}

Ref< IProgram > RenderSystemVk::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	Ref< const ProgramResourceVk > resource = dynamic_type_cast< const ProgramResourceVk* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramVk > program = new ProgramVk();
	if (!program->create(resource))
		return 0;

	return program;
}

Ref< IProgramCompiler > RenderSystemVk::createProgramCompiler() const
{
	return new ProgramCompilerVk();
}

Ref< ITimeQuery > RenderSystemVk::createTimeQuery() const
{
	Ref< TimeQueryVk > timeQuery = new TimeQueryVk();
	if (timeQuery->create())
		return timeQuery;
	else
		return 0;
}

void RenderSystemVk::purge()
{
}

void RenderSystemVk::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

	}
}
