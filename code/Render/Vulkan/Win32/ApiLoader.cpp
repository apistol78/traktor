#include "Core/Log/Log.h"
#include "Render/Vulkan/Win32/ApiLoader.h"

namespace traktor
{
	namespace render
	{
		namespace
		{
		
HMODULE s_hVulkanModule = 0;

		}

PFN_vkCreateInstance vkCreateInstance = 0;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = 0;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = 0;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = 0;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = 0;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = 0;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = 0;
PFN_vkCreateDevice vkCreateDevice = 0;
PFN_vkGetDeviceQueue vkGetDeviceQueue = 0;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = 0;
PFN_vkCreateCommandPool vkCreateCommandPool = 0;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = 0;
PFN_vkCreateShaderModule vkCreateShaderModule = 0;
PFN_vkCreateFence vkCreateFence = 0;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer = 0;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier = 0;
PFN_vkEndCommandBuffer vkEndCommandBuffer = 0;
PFN_vkQueueSubmit vkQueueSubmit = 0;
PFN_vkWaitForFences vkWaitForFences = 0;
PFN_vkResetFences vkResetFences = 0;
PFN_vkResetCommandBuffer vkResetCommandBuffer = 0;
PFN_vkCreateImageView vkCreateImageView = 0;
PFN_vkCreateBuffer vkCreateBuffer = 0;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = 0;
PFN_vkAllocateMemory vkAllocateMemory = 0;
PFN_vkMapMemory vkMapMemory = 0;
PFN_vkUnmapMemory vkUnmapMemory = 0;
PFN_vkBindBufferMemory vkBindBufferMemory = 0;
PFN_vkCreateSemaphore vkCreateSemaphore = 0;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass = 0;
PFN_vkDestroySemaphore vkDestroySemaphore = 0;
PFN_vkDestroyFence vkDestroyFence = 0;
PFN_vkCreateImage vkCreateImage = 0;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements = 0;
PFN_vkBindImageMemory vkBindImageMemory = 0;
PFN_vkCreateRenderPass vkCreateRenderPass = 0;
PFN_vkCreateFramebuffer vkCreateFramebuffer = 0;

PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = 0;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = 0;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = 0;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = 0;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = 0;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = 0;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = 0;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = 0;
PFN_vkQueuePresentKHR vkQueuePresentKHR = 0;

bool initializeVulkanApi()
{
	s_hVulkanModule = LoadLibrary(L"vulkan-1.dll");
	if (s_hVulkanModule == NULL)
	{
		log::error << L"Failed to load Vulkan library." << Endl;
		return false;
	}

	vkCreateInstance = (PFN_vkCreateInstance)GetProcAddress(s_hVulkanModule, "vkCreateInstance");
	if (vkCreateInstance == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateInstance\"." << Endl;
		return false;
	}

	vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)GetProcAddress(s_hVulkanModule, "vkEnumerateInstanceLayerProperties");
	if (vkEnumerateInstanceLayerProperties == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkEnumerateInstanceLayerProperties\"." << Endl;
		return false;
	}

	vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)GetProcAddress(s_hVulkanModule, "vkEnumerateInstanceExtensionProperties");
	if (vkEnumerateInstanceExtensionProperties == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkEnumerateInstanceExtensionProperties\"." << Endl;
		return false;
	}

	vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)GetProcAddress(s_hVulkanModule, "vkEnumeratePhysicalDevices");
	if (vkEnumeratePhysicalDevices == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkEnumeratePhysicalDevices\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)GetProcAddress(s_hVulkanModule, "vkGetPhysicalDeviceProperties");
	if (vkGetPhysicalDeviceProperties == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetPhysicalDeviceProperties\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)GetProcAddress(s_hVulkanModule, "vkGetPhysicalDeviceQueueFamilyProperties");
	if (vkGetPhysicalDeviceQueueFamilyProperties == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetPhysicalDeviceQueueFamilyProperties\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)GetProcAddress(s_hVulkanModule, "vkGetPhysicalDeviceMemoryProperties");
	if (vkGetPhysicalDeviceMemoryProperties == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetPhysicalDeviceMemoryProperties\"." << Endl;
		return false;
	}

	vkCreateDevice = (PFN_vkCreateDevice)GetProcAddress(s_hVulkanModule, "vkCreateDevice");
	if (vkCreateDevice == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateDevice\"." << Endl;
		return false;
	}

	vkGetDeviceQueue = (PFN_vkGetDeviceQueue)GetProcAddress(s_hVulkanModule, "vkGetDeviceQueue");
	if (vkGetDeviceQueue == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetDeviceQueue\"." << Endl;
		return false;
	}

	vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(s_hVulkanModule, "vkGetInstanceProcAddr");
	if (vkGetInstanceProcAddr == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetInstanceProcAddr\"." << Endl;
		return false;
	}

	vkCreateCommandPool = (PFN_vkCreateCommandPool)GetProcAddress(s_hVulkanModule, "vkCreateCommandPool");
	if (vkCreateCommandPool == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateCommandPool\"." << Endl;
		return false;
	}

	vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)GetProcAddress(s_hVulkanModule, "vkAllocateCommandBuffers");
	if (vkAllocateCommandBuffers == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkAllocateCommandBuffers\"." << Endl;
		return false;
	}

	vkCreateShaderModule = (PFN_vkCreateShaderModule)GetProcAddress(s_hVulkanModule, "vkCreateShaderModule");
	if (vkCreateShaderModule == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateShaderModule\"." << Endl;
		return false;
	}

	vkCreateFence = (PFN_vkCreateFence)GetProcAddress(s_hVulkanModule, "vkCreateFence");
	if (vkCreateFence == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateFence\"." << Endl;
		return false;
	}

	vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)GetProcAddress(s_hVulkanModule, "vkBeginCommandBuffer");
	if (vkBeginCommandBuffer == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkBeginCommandBuffer\"." << Endl;
		return false;
	}

	vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)GetProcAddress(s_hVulkanModule, "vkCmdPipelineBarrier");
	if (vkCmdPipelineBarrier == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdPipelineBarrier\"." << Endl;
		return false;
	}

	vkEndCommandBuffer = (PFN_vkEndCommandBuffer)GetProcAddress(s_hVulkanModule, "vkEndCommandBuffer");
	if (vkEndCommandBuffer == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkEndCommandBuffer\"." << Endl;
		return false;
	}

	vkQueueSubmit = (PFN_vkQueueSubmit)GetProcAddress(s_hVulkanModule, "vkQueueSubmit");
	if (vkQueueSubmit == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkQueueSubmit\"." << Endl;
		return false;
	}

	vkWaitForFences = (PFN_vkWaitForFences)GetProcAddress(s_hVulkanModule, "vkWaitForFences");
	if (vkWaitForFences == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkWaitForFences\"." << Endl;
		return false;
	}

	vkResetFences = (PFN_vkResetFences)GetProcAddress(s_hVulkanModule, "vkResetFences");
	if (vkResetFences == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkResetFences\"." << Endl;
		return false;
	}

	vkResetCommandBuffer = (PFN_vkResetCommandBuffer)GetProcAddress(s_hVulkanModule, "vkResetCommandBuffer");
	if (vkResetCommandBuffer == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkResetCommandBuffer\"." << Endl;
		return false;
	}

	vkCreateImageView = (PFN_vkCreateImageView)GetProcAddress(s_hVulkanModule, "vkCreateImageView");
	if (vkCreateImageView == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateImageView\"." << Endl;
		return false;
	}

	vkCreateBuffer = (PFN_vkCreateBuffer)GetProcAddress(s_hVulkanModule, "vkCreateBuffer");
	if (vkCreateBuffer == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateBuffer\"." << Endl;
		return false;
	}

	vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)GetProcAddress(s_hVulkanModule, "vkGetBufferMemoryRequirements");
	if (vkGetBufferMemoryRequirements == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetBufferMemoryRequirements\"." << Endl;
		return false;
	}

	vkAllocateMemory = (PFN_vkAllocateMemory)GetProcAddress(s_hVulkanModule, "vkAllocateMemory");
	if (vkAllocateMemory == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkAllocateMemory\"." << Endl;
		return false;
	}

	vkMapMemory = (PFN_vkMapMemory)GetProcAddress(s_hVulkanModule, "vkMapMemory");
	if (vkMapMemory == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkMapMemory\"." << Endl;
		return false;
	}

	vkUnmapMemory = (PFN_vkUnmapMemory)GetProcAddress(s_hVulkanModule, "vkUnmapMemory");
	if (vkUnmapMemory == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkUnmapMemory\"." << Endl;
		return false;
	}

	vkBindBufferMemory = (PFN_vkBindBufferMemory)GetProcAddress(s_hVulkanModule, "vkBindBufferMemory");
	if (vkBindBufferMemory == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkBindBufferMemory\"." << Endl;
		return false;
	}

	vkCreateSemaphore = (PFN_vkCreateSemaphore)GetProcAddress(s_hVulkanModule, "vkCreateSemaphore");
	if (vkCreateSemaphore == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateSemaphore\"." << Endl;
		return false;
	}

	vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)GetProcAddress(s_hVulkanModule, "vkCmdBeginRenderPass");
	if (vkCmdBeginRenderPass == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdBeginRenderPass\"." << Endl;
		return false;
	}

	vkDestroySemaphore = (PFN_vkDestroySemaphore)GetProcAddress(s_hVulkanModule, "vkDestroySemaphore");
	if (vkDestroySemaphore == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroySemaphore\"." << Endl;
		return false;
	}

	vkDestroyFence = (PFN_vkDestroyFence)GetProcAddress(s_hVulkanModule, "vkDestroyFence");
	if (vkDestroyFence == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroyFence\"." << Endl;
		return false;
	}

	vkCreateImage = (PFN_vkCreateImage)GetProcAddress(s_hVulkanModule, "vkCreateImage");
	if (vkCreateImage == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateImage\"." << Endl;
		return false;
	}

	vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)GetProcAddress(s_hVulkanModule, "vkGetImageMemoryRequirements");
	if (vkGetImageMemoryRequirements == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetImageMemoryRequirements\"." << Endl;
		return false;
	}

	vkBindImageMemory = (PFN_vkBindImageMemory)GetProcAddress(s_hVulkanModule, "vkBindImageMemory");
	if (vkBindImageMemory == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkBindImageMemory\"." << Endl;
		return false;
	}

	vkCreateRenderPass = (PFN_vkCreateRenderPass)GetProcAddress(s_hVulkanModule, "vkCreateRenderPass");
	if (vkCreateRenderPass == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateRenderPass\"." << Endl;
		return false;
	}

	vkCreateFramebuffer = (PFN_vkCreateFramebuffer)GetProcAddress(s_hVulkanModule, "vkCreateFramebuffer");
	if (vkCreateFramebuffer == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateFramebuffer\"." << Endl;
		return false;
	}

	return true;
}

bool initializeVulkanExtensions(VkInstance instance)
{
	*(void**)&vkCreateWin32SurfaceKHR = vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	if (vkCreateWin32SurfaceKHR == NULL)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkCreateWin32SurfaceKHR\"." << Endl;
		return false;
	}

	*(void**)&vkGetPhysicalDeviceSurfaceSupportKHR = vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
	if (vkGetPhysicalDeviceSurfaceSupportKHR == NULL)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkGetPhysicalDeviceSurfaceSupportKHR\"." << Endl;
		return false;
	}

	*(void**)&vkGetPhysicalDeviceSurfaceFormatsKHR = vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
	if (vkGetPhysicalDeviceSurfaceFormatsKHR == NULL)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkGetPhysicalDeviceSurfaceFormatsKHR\"." << Endl;
		return false;
	}

	*(void**)&vkGetPhysicalDeviceSurfaceCapabilitiesKHR = vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR == NULL)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkGetPhysicalDeviceSurfaceCapabilitiesKHR\"." << Endl;
		return false;
	}

	*(void**)&vkGetPhysicalDeviceSurfacePresentModesKHR = vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
	if (vkGetPhysicalDeviceSurfacePresentModesKHR == NULL)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkGetPhysicalDeviceSurfacePresentModesKHR\"." << Endl;
		return false;
	}

	*(void**)&vkCreateSwapchainKHR = vkGetInstanceProcAddr(instance, "vkCreateSwapchainKHR");
	if (vkCreateSwapchainKHR == NULL)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkCreateSwapchainKHR\"." << Endl;
		return false;
	}

	*(void**)&vkGetSwapchainImagesKHR = vkGetInstanceProcAddr(instance, "vkGetSwapchainImagesKHR");
	if (vkGetSwapchainImagesKHR == NULL)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkGetSwapchainImagesKHR\"." << Endl;
		return false;
	}

	*(void**)&vkAcquireNextImageKHR = vkGetInstanceProcAddr(instance, "vkAcquireNextImageKHR");
	if (vkAcquireNextImageKHR == NULL)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkAcquireNextImageKHR\"." << Endl;
		return false;
	}

	*(void**)&vkQueuePresentKHR = vkGetInstanceProcAddr(instance, "vkQueuePresentKHR");
	if (vkQueuePresentKHR == NULL)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkQueuePresentKHR\"." << Endl;
		return false;
	}

	return true;
}

void finalizeVulkanApi()
{
}

	}
}
