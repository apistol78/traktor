#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/Vulkan/ContextVk.h"
#include "Render/Vulkan/CubeTextureVk.h"
#include "Render/Vulkan/IndexBufferVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/ProgramCompilerVk.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/RenderSystemVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/RenderViewVk.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/TimeQueryVk.h"
#include "Render/Vulkan/VertexBufferVk.h"
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
:	m_instance(0)
,	m_surface(0)
,	m_physicalDevice(0)
,	m_physicalDeviceQueueIndex(0)
,	m_device(0)
,	m_haveValidationLayer(false)
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

	log::info << IncreaseIndent;
	for (uint32_t i = 0; i < layerCount; ++i)
	{
		log::info << i << L": " << mbstows(layersAvailable[i].layerName) << Endl;
		if (strcmp(layersAvailable[i].layerName, c_validationLayerNames[0]) == 0)
			m_haveValidationLayer = true;
	}
	log::info << DecreaseIndent;
	if (!m_haveValidationLayer)
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

	if (m_haveValidationLayer)
	{
		instanceInfo.enabledLayerCount = sizeof_array(c_validationLayerNames);
		instanceInfo.ppEnabledLayerNames = c_validationLayerNames;
	}
	
	// Create Vulkan instance.
	VkInstance vkInstance = 0;
	if (vkCreateInstance(&instanceInfo, 0, &m_instance) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan instance." << Endl;
		return false;
	}

	// Load Vulkan extensions.
	if (!initializeVulkanExtensions(m_instance))
	{
		log::error << L"Failed to create Vulkan; failed to load extensions." << Endl;
		return false;
	}


	// Create Windows renderable surface.
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
    surfaceCreateInfo.hwnd = (HWND)*m_window;

    if (vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, NULL, &m_surface) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create Win32 renderable surface." << Endl;
		return false;
	}

	// Find Vulkan physical device.
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, 0);
	if (physicalDeviceCount == 0)
	{
		log::error << L"Failed to create Vulkan; no physical devices." << Endl;
		return false;
	}
	log::info << physicalDeviceCount << L" physical device(s) supporting Vulkan found." << Endl;

	AutoArrayPtr< VkPhysicalDevice > physicalDevices(new VkPhysicalDevice[physicalDeviceCount]);
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.ptr());

	log::info << IncreaseIndent;
	for (uint32_t i = 0; i < physicalDeviceCount; ++i)
	{
		VkPhysicalDeviceProperties deviceProperties = {};
		vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
 
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, 0);
		log::info << i << L": " << mbstows(deviceProperties.deviceName) << L", " << queueFamilyCount << L" physical device family properties."<< Endl;

		AutoArrayPtr< VkQueueFamilyProperties > queueFamilyProperties(new VkQueueFamilyProperties[queueFamilyCount]);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, queueFamilyProperties.ptr());
 
		log::info << IncreaseIndent;
		for (uint32_t j = 0; j < queueFamilyCount; ++j)
		{
 			VkBool32 supportsPresent;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], j, m_surface, &supportsPresent);

			log::info << j << L": supportsPresent = " << (supportsPresent ? L"yes" : L"no") << Endl;
 
			if(supportsPresent && (queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				m_physicalDevice = physicalDevices[i];
				//context.physicalDeviceProperties = deviceProperties;
				m_physicalDeviceQueueIndex = j;
			}
		}
		log::info << DecreaseIndent;
	}
	log::info << DecreaseIndent;

	// Create device.
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_physicalDeviceQueueIndex;
    queueCreateInfo.queueCount = 1;
    float queuePriorities[] = { 1.0f };
    queueCreateInfo.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceInfo.enabledLayerCount = 0;
    deviceInfo.ppEnabledLayerNames = 0;

	if (m_haveValidationLayer)
	{
		deviceInfo.enabledLayerCount = sizeof_array(c_validationLayerNames);
		deviceInfo.ppEnabledLayerNames = c_validationLayerNames;
	}
    
    deviceInfo.enabledExtensionCount = sizeof_array(c_deviceExtensions);
    deviceInfo.ppEnabledExtensionNames = c_deviceExtensions;

    VkPhysicalDeviceFeatures features = {};
    features.shaderClipDistance = VK_TRUE;
    deviceInfo.pEnabledFeatures = &features;

    if (vkCreateDevice(m_physicalDevice, &deviceInfo, 0, &m_device) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create device." << Endl;
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
#if defined(_WIN32)
	uint32_t count = 0;

	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	while (EnumDisplaySettings(NULL, count, &dmgl))
		++count;

	return count;
#else
	return 0;
#endif
}

DisplayMode RenderSystemVk::getDisplayMode(uint32_t index) const
{
#if defined(_WIN32)
	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	EnumDisplaySettings(NULL, index, &dmgl);

	DisplayMode dm;
	dm.width = dmgl.dmPelsWidth;
	dm.height = dmgl.dmPelsHeight;
	dm.refreshRate = (uint16_t)dmgl.dmDisplayFrequency;
	dm.colorBits = (uint16_t)dmgl.dmBitsPerPel;
	return dm;
#else
	return DisplayMode();
#endif
}

DisplayMode RenderSystemVk::getCurrentDisplayMode() const
{
#if defined(_WIN32)
	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmgl);

	DisplayMode dm;
	dm.width = dmgl.dmPelsWidth;
	dm.height = dmgl.dmPelsHeight;
	dm.refreshRate = (uint16_t)dmgl.dmDisplayFrequency;
	dm.colorBits = (uint16_t)dmgl.dmBitsPerPel;
	return dm;
#else
	return DisplayMode();
#endif
}

float RenderSystemVk::getDisplayAspectRatio() const
{
	return 1.0f;
}

Ref< IRenderView > RenderSystemVk::createRenderView(const RenderViewDefaultDesc& desc)
{
	VkSwapchainKHR swapChain = 0;
	VkQueue presentQueue = 0;
	VkCommandPool commandPool = 0;
	VkCommandBuffer setupCmdBuffer = 0;
	VkCommandBuffer drawCmdBuffer = 0;
	VkImage depthImage = 0;
	VkImageView depthImageView = 0;
	VkRenderPass renderPass = 0;


	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - Vulkan Renderer");
	m_window->setWindowedStyle(
		desc.displayMode.width,
		desc.displayMode.height
	);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
	log::info << formatCount << L" surface formats available." << Endl;

	AutoArrayPtr< VkSurfaceFormatKHR > surfaceFormats(new VkSurfaceFormatKHR[formatCount]);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, surfaceFormats.ptr());
 
	// If the format list includes just one entry of VK_FORMAT_UNDEFINED, the surface has
	// no preferred format. Otherwise, at least one supported format will be returned.
	VkFormat colorFormat;
	if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
		colorFormat = VK_FORMAT_B8G8R8_UNORM;
	else
		colorFormat = surfaceFormats[0].format;

	VkColorSpaceKHR colorSpace;
	colorSpace = surfaceFormats[0].colorSpace;

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCapabilities);
 
	// we are effectively looking for double-buffering:
	// if surfaceCapabilities.maxImageCount == 0 there is actually no limit on the number of images! 
	uint32_t desiredImageCount = 2;
	if (desiredImageCount < surfaceCapabilities.minImageCount)
		desiredImageCount = surfaceCapabilities.minImageCount;
	else if( surfaceCapabilities.maxImageCount != 0 && desiredImageCount > surfaceCapabilities.maxImageCount)
		desiredImageCount = surfaceCapabilities.maxImageCount;

 
	VkExtent2D surfaceResolution =  surfaceCapabilities.currentExtent;
	if (surfaceResolution.width == -1)
	{
		surfaceResolution.width = desc.displayMode.width;
		surfaceResolution.height = desc.displayMode.height;
	}
	//else
	//{
	//	context.width = surfaceResolution.width;
	//	context.height = surfaceResolution.height;
	//}
 
	VkSurfaceTransformFlagBitsKHR preTransform = surfaceCapabilities.currentTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);
	log::info << presentModeCount << L" present modes available." << Endl;

	AutoArrayPtr< VkPresentModeKHR > presentModes(new VkPresentModeKHR[presentModeCount]);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.ptr());
 
	VkPresentModeKHR presentationMode = VK_PRESENT_MODE_FIFO_KHR;   // always supported.
	for (uint32_t i = 0; i < presentModeCount; ++i)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}   
	}

	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = m_surface;
	swapChainCreateInfo.minImageCount = desiredImageCount;
	swapChainCreateInfo.imageFormat = colorFormat;
	swapChainCreateInfo.imageColorSpace = colorSpace;
	swapChainCreateInfo.imageExtent = surfaceResolution;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCreateInfo.preTransform = preTransform;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode = presentationMode;
	swapChainCreateInfo.clipped = true;
	if (vkCreateSwapchainKHR(m_device, &swapChainCreateInfo, NULL, &swapChain) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create swap chain." << Endl;
		return false;
	}
	
	// Get device submit queue.
	vkGetDeviceQueue(m_device, m_physicalDeviceQueueIndex, 0, &presentQueue);

	// Create command pool.
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = m_physicalDeviceQueueIndex;
	if (vkCreateCommandPool(m_device, &commandPoolCreateInfo, 0, &commandPool) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create command pool." << Endl;
		return false;
	}

	// Create command buffers from pool.
	VkCommandBufferAllocateInfo commandBufferAllocationInfo = {};
	commandBufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocationInfo.commandPool = commandPool;
	commandBufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocationInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(m_device, &commandBufferAllocationInfo, &setupCmdBuffer) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to allocate setup command buffer." << Endl;
		return false;
	}
	if (vkAllocateCommandBuffers(m_device, &commandBufferAllocationInfo, &drawCmdBuffer) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to allocate draw command buffer." << Endl;
		return false;
	}


	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(m_device, swapChain, &imageCount, nullptr);
	log::info << imageCount << L" images in swap chain." << Endl;

	AlignedVector< VkImage > presentImages(imageCount);
	vkGetSwapchainImagesKHR(m_device, swapChain, &imageCount, presentImages.ptr());
 
	VkImageViewCreateInfo presentImagesViewCreateInfo = {};
	presentImagesViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	presentImagesViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	presentImagesViewCreateInfo.format = colorFormat;
	presentImagesViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	presentImagesViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	presentImagesViewCreateInfo.subresourceRange.baseMipLevel = 0;
	presentImagesViewCreateInfo.subresourceRange.levelCount = 1;
	presentImagesViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	presentImagesViewCreateInfo.subresourceRange.layerCount = 1;



	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
 
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence submitFence;
	vkCreateFence(m_device, &fenceCreateInfo, nullptr, &submitFence);



	AlignedVector< VkImageView > presentImageViews(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		// complete VkImageViewCreateInfo with image i:
		presentImagesViewCreateInfo.image = presentImages[i];
 
		// start recording on our setup command buffer:
		vkBeginCommandBuffer(setupCmdBuffer, &beginInfo);
 
		VkImageMemoryBarrier layoutTransitionBarrier = {};
		layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		layoutTransitionBarrier.srcAccessMask = 0; 
		layoutTransitionBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		layoutTransitionBarrier.image = presentImages[i];
		VkImageSubresourceRange resourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		layoutTransitionBarrier.subresourceRange = resourceRange;
 
		vkCmdPipelineBarrier(
			setupCmdBuffer, 
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
			0,
			0, nullptr,
			0, nullptr, 
			1, &layoutTransitionBarrier
		);
 
		vkEndCommandBuffer(setupCmdBuffer);
 
		// submitting code to the queue:
		VkPipelineStageFlags waitStageMask[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = NULL;
		submitInfo.pWaitDstStageMask = waitStageMask;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &setupCmdBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = NULL;
		if (vkQueueSubmit(presentQueue, 1, &submitInfo, submitFence) != VK_SUCCESS)
		{
			return 0;
		}
 
		// waiting for it to finish:
		vkWaitForFences(m_device, 1, &submitFence, VK_TRUE, UINT64_MAX);
		vkResetFences(m_device, 1, &submitFence);
 
		vkResetCommandBuffer(setupCmdBuffer, 0);
 
		// create the image view:
		if (vkCreateImageView(m_device, &presentImagesViewCreateInfo, NULL, &presentImageViews[i]) != VK_SUCCESS)
		{
			return 0;
		}
	}


	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);
 
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_D16_UNORM;
	imageCreateInfo.extent = { desc.displayMode.width, desc.displayMode.height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
 	if (vkCreateImage(m_device, &imageCreateInfo, nullptr, &depthImage) != VK_SUCCESS)
		return 0;

	VkMemoryRequirements memoryRequirements = {};
	vkGetImageMemoryRequirements(m_device, depthImage, &memoryRequirements);
 
	VkMemoryAllocateInfo imageAllocateInfo = {};
	imageAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageAllocateInfo.allocationSize = memoryRequirements.size;
 
	// memoryTypeBits is a bitfield where if bit i is set, it means that 
	// the VkMemoryType i of the VkPhysicalDeviceMemoryProperties structure 
	// satisfies the memory requirements:
	uint32_t memoryTypeBits = memoryRequirements.memoryTypeBits;
	VkMemoryPropertyFlags desiredMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	for (uint32_t i = 0; i < 32; ++i)
	{
		VkMemoryType memoryType = memoryProperties.memoryTypes[i];
		if (memoryTypeBits & 1)
		{
			if ((memoryType.propertyFlags & desiredMemoryFlags) == desiredMemoryFlags)
			{
				imageAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		memoryTypeBits = memoryTypeBits >> 1;
	}
 
	VkDeviceMemory imageMemory = {};
	if (vkAllocateMemory(m_device, &imageAllocateInfo, nullptr, &imageMemory) != VK_SUCCESS)
		return 0;
 
	if (vkBindImageMemory(m_device, depthImage, imageMemory, 0) != VK_SUCCESS)
		return 0;

	//VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(setupCmdBuffer, &beginInfo);
 
	VkImageMemoryBarrier layoutTransitionBarrier = {};
	layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutTransitionBarrier.srcAccessMask = 0;
	layoutTransitionBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.image = depthImage;
	VkImageSubresourceRange resourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
	layoutTransitionBarrier.subresourceRange = resourceRange;

	vkCmdPipelineBarrier(
		setupCmdBuffer, 
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
		0,
		0, nullptr,
		0, nullptr, 
		1, &layoutTransitionBarrier
	);
 
	vkEndCommandBuffer(setupCmdBuffer);
 
	VkPipelineStageFlags waitStageMask[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = waitStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &setupCmdBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;
	if (vkQueueSubmit(presentQueue, 1, &submitInfo, submitFence) != VK_SUCCESS)
		return 0;
 
	vkWaitForFences(m_device, 1, &submitFence, VK_TRUE, UINT64_MAX);
	vkResetFences(m_device, 1, &submitFence);
	vkResetCommandBuffer(setupCmdBuffer, 0);


	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = depthImage;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = imageCreateInfo.format;
	imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
 	if (vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &depthImageView) != VK_SUCCESS)
		return 0;


	VkAttachmentDescription passAttachments[2] = { };
	passAttachments[0].format = colorFormat;
	passAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	passAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	passAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	passAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	passAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	passAttachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
 
	passAttachments[1].format = VK_FORMAT_D16_UNORM;
	passAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	passAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	passAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	passAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	passAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
 
	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
 
	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;
 
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 2;
	renderPassCreateInfo.pAttachments = passAttachments;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	if (vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
		return 0;



	VkImageView frameBufferAttachments[2];
	frameBufferAttachments[1] = depthImageView;
 
	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.renderPass = renderPass;
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments = frameBufferAttachments;
	frameBufferCreateInfo.width = desc.displayMode.width;
	frameBufferCreateInfo.height = desc.displayMode.height;
	frameBufferCreateInfo.layers = 1;
 
	// create a framebuffer per swap chain imageView:
	AlignedVector< VkFramebuffer > frameBuffers(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		frameBufferAttachments[0] = presentImageViews[ i ];
		if (vkCreateFramebuffer(m_device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
			return 0;
	}


#if defined(_WIN32)
	Ref< RenderViewVk > renderView = new RenderViewVk(
		m_window,
		m_device,
		swapChain,
		presentQueue,
		drawCmdBuffer,
		presentImages,
		depthImage,
		renderPass,
		frameBuffers
	);
#else
	Ref< RenderViewVk > renderView = new RenderViewVk(m_device);
#endif

	return renderView;
}

Ref< IRenderView > RenderSystemVk::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	return 0;
}

Ref< VertexBuffer > RenderSystemVk::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	VkBuffer vertexBuffer;

	VkBufferCreateInfo vertexBufferInfo = {};
	vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.size = bufferSize;
	vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(m_device, &vertexBufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS)
		return 0;

	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

	VkMemoryRequirements vertexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(m_device, vertexBuffer, &vertexBufferMemoryRequirements);
 
	VkMemoryAllocateInfo bufferAllocateInfo = {};
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferAllocateInfo.allocationSize = vertexBufferMemoryRequirements.size;
 
	uint32_t vertexMemoryTypeBits = vertexBufferMemoryRequirements.memoryTypeBits;
	VkMemoryPropertyFlags vertexDesiredMemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	for (uint32_t i = 0; i < 32; ++i)
	{
		VkMemoryType memoryType = memoryProperties.memoryTypes[i];
		if (vertexMemoryTypeBits & 1)
		{
			if ((memoryType.propertyFlags & vertexDesiredMemoryFlags) == vertexDesiredMemoryFlags)
			{
				bufferAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vertexMemoryTypeBits = vertexMemoryTypeBits >> 1;
	}
 
	VkDeviceMemory vertexBufferMemory;
	if (vkAllocateMemory(m_device, &bufferAllocateInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
		return 0;

	return new VertexBufferVk(bufferSize, m_device, vertexBuffer, vertexBufferMemory);
}

Ref< IndexBuffer > RenderSystemVk::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	VkBuffer indexBuffer;

	VkBufferCreateInfo indexBufferInfo = {};
	indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexBufferInfo.size = bufferSize;
	indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(m_device, &indexBufferInfo, nullptr, &indexBuffer) != VK_SUCCESS)
		return 0;

	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

	VkMemoryRequirements indexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(m_device, indexBuffer, &indexBufferMemoryRequirements);
 
	VkMemoryAllocateInfo bufferAllocateInfo = {};
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferAllocateInfo.allocationSize = indexBufferMemoryRequirements.size;
 
	uint32_t vertexMemoryTypeBits = indexBufferMemoryRequirements.memoryTypeBits;
	VkMemoryPropertyFlags vertexDesiredMemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	for (uint32_t i = 0; i < 32; ++i)
	{
		VkMemoryType memoryType = memoryProperties.memoryTypes[i];
		if (vertexMemoryTypeBits & 1)
		{
			if ((memoryType.propertyFlags & vertexDesiredMemoryFlags) == vertexDesiredMemoryFlags)
			{
				bufferAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vertexMemoryTypeBits = vertexMemoryTypeBits >> 1;
	}
 
	VkDeviceMemory indexBufferMemory;
	if (vkAllocateMemory(m_device, &bufferAllocateInfo, nullptr, &indexBufferMemory) != VK_SUCCESS)
		return 0;

	return new IndexBufferVk(indexType, bufferSize, m_device, indexBuffer, indexBufferMemory);
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

	Ref< ProgramVk > program = new ProgramVk(m_device);
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
