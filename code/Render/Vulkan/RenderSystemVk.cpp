#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/Vulkan/ContextVk.h"
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
		namespace
		{

const char* c_validationLayerNames[] = { "VK_LAYER_LUNARG_standard_validation" };
const char* c_extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report" };
const char* c_deviceExtensions[] = { "VK_KHR_swapchain" };

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemVk", 0, RenderSystemVk, IRenderSystem)

RenderSystemVk::RenderSystemVk()
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
	log::info << layerCount << L" vulkan layer(s)" << Endl;
 
	AutoArrayPtr< VkLayerProperties > layersAvailable(new VkLayerProperties[layerCount]);
	vkEnumerateInstanceLayerProperties(&layerCount, layersAvailable.ptr());

	bool foundValidation = false;
	for (uint32_t i = 0; i < layerCount; ++i)
	{
		log::info << i << L". \"" << mbstows(layersAvailable[i].layerName) << L"\"" << Endl;

		if (strcmp(layersAvailable[i].layerName, c_validationLayerNames[0]) == 0)
			foundValidation = true;
	}
	if (!foundValidation)
		log::warning << L"No Vulkan validation layer found." << Endl;

	//uint32_t extensionCount = 0;
	//vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0);
	//log::info << extensionCount << L" vulkan extension(s)" << Endl;

	//AutoArrayPtr< VkExtensionProperties > extensionsAvailable(new VkExtensionProperties[extensionCount]);
	//vkEnumerateInstanceExtensionProperties(0, &extensionCount, extensionsAvailable.ptr());

	//uint32_t foundExtensions = 0;
	//for (int i = 0; i < extensionCount; ++i)
	//{
	//	if(strcmp(extensionsAvailable[i].extensionName, extensions[i]) == 0)
	//		foundExtensions++;
	//}
	//if (foundExtensions != sizeof_array(extensions))
	//{
	//	log::error << L"Failed to create Vulkan; required extensions missing." << Endl;
	//	return false;
	//}

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
	instanceInfo.enabledExtensionCount = sizeof_array(c_extensions);
	instanceInfo.ppEnabledExtensionNames = c_extensions;

	if (foundValidation)
	{
		instanceInfo.enabledLayerCount = sizeof_array(c_validationLayerNames);
		instanceInfo.ppEnabledLayerNames = c_validationLayerNames;
	}
	
	// Create Vulkan instance.
	VkInstance vkInstance = 0;
	if (vkCreateInstance(&instanceInfo, 0, &vkInstance) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan instance." << Endl;
		return false;
	}

	// Load Vulkan extensions.
	if (!initializeVulkanExtensions(vkInstance))
	{
		log::error << L"Failed to create Vulkan; failed to load extensions." << Endl;
		return false;
	}

	// Create Windows renderable surface.
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
    surfaceCreateInfo.hwnd = (HWND)*m_window;

	VkSurfaceKHR vkSurface = 0;
    if (vkCreateWin32SurfaceKHR(vkInstance, &surfaceCreateInfo, NULL, &vkSurface) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create Win32 renderable surface." << Endl;
		return false;
	}

	// Find Vulkan physical device.
	VkPhysicalDevice vkPhysicalDevice = 0;
	uint32_t vkPhysicalDeviceQueueIndex = 0;

	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, 0);
	if (physicalDeviceCount == 0)
	{
		log::error << L"Failed to create Vulkan; no physical devices." << Endl;
		return false;
	}
	log::info << physicalDeviceCount << L" physical device(s) supporting Vulkan found." << Endl;

	AutoArrayPtr< VkPhysicalDevice > physicalDevices(new VkPhysicalDevice[physicalDeviceCount]);
	vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices.ptr());

	for (uint32_t i = 0; i < physicalDeviceCount; ++i)
	{
		VkPhysicalDeviceProperties deviceProperties = {};
		vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);

		log::info << i << L". \"" << mbstows(deviceProperties.deviceName) << L"\"" << Endl;
 
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, 0);

		log::info << i << L".   " << queueFamilyCount << L" physical device family properties." << Endl;

		AutoArrayPtr< VkQueueFamilyProperties > queueFamilyProperties(new VkQueueFamilyProperties[queueFamilyCount]);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, queueFamilyProperties.ptr());
 
		for (uint32_t j = 0; j < queueFamilyCount; ++j)
		{
 			VkBool32 supportsPresent;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], j, vkSurface, &supportsPresent);

			log::info << i << L".   " << j << L". supportsPresent = " << (supportsPresent ? L"yes" : L"no") << Endl;
 
			if(supportsPresent && (queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				vkPhysicalDevice = physicalDevices[i];
				//context.physicalDeviceProperties = deviceProperties;
				vkPhysicalDeviceQueueIndex = j;
			}
		}
	}

	// Create shared device.
	VkDevice vkDevice = 0;

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memoryProperties);

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = vkPhysicalDeviceQueueIndex;
    queueCreateInfo.queueCount = 1;
    float queuePriorities[] = { 1.0f };
    queueCreateInfo.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceInfo.enabledLayerCount = 0;
    deviceInfo.ppEnabledLayerNames = 0;

	if (foundValidation)
	{
		deviceInfo.enabledLayerCount = sizeof_array(c_validationLayerNames);
		deviceInfo.ppEnabledLayerNames = c_validationLayerNames;
	}
    
    deviceInfo.enabledExtensionCount = sizeof_array(c_deviceExtensions);
    deviceInfo.ppEnabledExtensionNames = c_deviceExtensions;

    VkPhysicalDeviceFeatures features = {};
    features.shaderClipDistance = VK_TRUE;
    deviceInfo.pEnabledFeatures = &features;

    if (vkCreateDevice(vkPhysicalDevice, &deviceInfo, 0, &vkDevice) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create device." << Endl;
		return false;
	}

	// Get device submit queue.
	VkQueue vkPresentQueue = 0;
	vkGetDeviceQueue(vkDevice, vkPhysicalDeviceQueueIndex, 0, &vkPresentQueue);

	// Create command buffers.
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = vkPhysicalDeviceQueueIndex;

	VkCommandPool vkCommandPool;
	if (vkCreateCommandPool(vkDevice, &commandPoolCreateInfo, 0, &vkCommandPool) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create command pool." << Endl;
		return false;
	}

	VkCommandBufferAllocateInfo commandBufferAllocationInfo = {};
	commandBufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocationInfo.commandPool = vkCommandPool;
	commandBufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocationInfo.commandBufferCount = 1;

	VkCommandBuffer vkSetupCmdBuffer = 0;
	if (vkAllocateCommandBuffers(vkDevice, &commandBufferAllocationInfo, &vkSetupCmdBuffer ) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to allocate setup command buffer." << Endl;
		return false;
	}

	VkCommandBuffer vkDrawCmdBuffer = 0;
	if (vkAllocateCommandBuffers(vkDevice, &commandBufferAllocationInfo, &vkDrawCmdBuffer ) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to allocate draw command buffer." << Endl;
		return false;
	}

	// Create shared context.
	m_sharedContext = new ContextVk(vkInstance, vkSurface, vkPhysicalDevice, vkDevice, vkPresentQueue);

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

	Ref< ProgramVk > program = new ProgramVk(m_sharedContext);
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
