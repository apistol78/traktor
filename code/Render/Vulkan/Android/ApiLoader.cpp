#include <dlfcn.h>
#include "Core/Log/Log.h"
#include "Render/Vulkan/Android/ApiLoader.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

void* s_hVulkanModule = nullptr;

		}

PFN_vkCreateInstance vkCreateInstance = nullptr;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = nullptr;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = nullptr;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = nullptr;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = nullptr;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = nullptr;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = nullptr;
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties = nullptr;
PFN_vkCreateDevice vkCreateDevice = nullptr;
PFN_vkGetDeviceQueue vkGetDeviceQueue = nullptr;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
PFN_vkCreateCommandPool vkCreateCommandPool = nullptr;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = nullptr;
PFN_vkCreateShaderModule vkCreateShaderModule = nullptr;
PFN_vkCreateFence vkCreateFence = nullptr;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer = nullptr;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier = nullptr;
PFN_vkEndCommandBuffer vkEndCommandBuffer = nullptr;
PFN_vkQueueSubmit vkQueueSubmit = nullptr;
PFN_vkWaitForFences vkWaitForFences = nullptr;
PFN_vkResetFences vkResetFences = nullptr;
PFN_vkResetCommandBuffer vkResetCommandBuffer = nullptr;
PFN_vkCreateImageView vkCreateImageView = nullptr;
PFN_vkCreateBuffer vkCreateBuffer = nullptr;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = nullptr;
PFN_vkAllocateMemory vkAllocateMemory = nullptr;
PFN_vkMapMemory vkMapMemory = nullptr;
PFN_vkUnmapMemory vkUnmapMemory = nullptr;
PFN_vkBindBufferMemory vkBindBufferMemory = nullptr;
PFN_vkCreateSemaphore vkCreateSemaphore = nullptr;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass = nullptr;
PFN_vkDestroySemaphore vkDestroySemaphore = nullptr;
PFN_vkDestroyFence vkDestroyFence = nullptr;
PFN_vkCreateImage vkCreateImage = nullptr;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements = nullptr;
PFN_vkBindImageMemory vkBindImageMemory = nullptr;
PFN_vkCreateRenderPass vkCreateRenderPass = nullptr;
PFN_vkCreateFramebuffer vkCreateFramebuffer = nullptr;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass = nullptr;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers = nullptr;
PFN_vkCmdDraw vkCmdDraw = nullptr;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer = nullptr;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed = nullptr;
PFN_vkCmdBindPipeline vkCmdBindPipeline = nullptr;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout = nullptr;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines = nullptr;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout = nullptr;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool = nullptr;
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets = nullptr;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets = nullptr;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets = nullptr;
PFN_vkDestroyPipeline vkDestroyPipeline = nullptr;
PFN_vkCreateEvent vkCreateEvent = nullptr;
PFN_vkDestroyEvent vkDestroyEvent = nullptr;
PFN_vkCmdSetEvent vkCmdSetEvent = nullptr;
PFN_vkGetEventStatus vkGetEventStatus = nullptr;
PFN_vkCmdExecuteCommands vkCmdExecuteCommands = nullptr;
PFN_vkResetDescriptorPool vkResetDescriptorPool = nullptr;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers = nullptr;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle = nullptr;
PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges = nullptr;
PFN_vkCreateSampler vkCreateSampler = nullptr;
PFN_vkQueueWaitIdle vkQueueWaitIdle = nullptr;
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage = nullptr;
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties = nullptr;
PFN_vkCmdSetViewport vkCmdSetViewport = nullptr;
PFN_vkFreeMemory vkFreeMemory = nullptr;
PFN_vkDestroyBuffer vkDestroyBuffer = nullptr;
PFN_vkCmdCopyImage vkCmdCopyImage = nullptr;
PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout = nullptr;
PFN_vkDestroyImage vkDestroyImage = nullptr;
PFN_vkCmdDispatch vkCmdDispatch = nullptr;
PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges = nullptr;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer = nullptr;
PFN_vkDestroyShaderModule vkDestroyShaderModule = nullptr;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout = nullptr;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout = nullptr;
PFN_vkDestroySampler vkDestroySampler = nullptr;
PFN_vkDestroyCommandPool vkDestroyCommandPool = nullptr;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool = nullptr;
PFN_vkCmdClearColorImage vkCmdClearColorImage = nullptr;
PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage = nullptr;
PFN_vkCmdSetStencilReference vkCmdSetStencilReference = nullptr;

PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = nullptr;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = nullptr;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = nullptr;
PFN_vkQueuePresentKHR vkQueuePresentKHR = nullptr;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = nullptr;
//PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
//PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;

bool initializeVulkanApi()
{
	s_hVulkanModule = dlopen("libvulkan.so", RTLD_LAZY | RTLD_GLOBAL);
	if (s_hVulkanModule == nullptr)
	{
		log::error << L"Failed to load Vulkan library." << Endl;
		return false;
	}

	vkCreateInstance = (PFN_vkCreateInstance)dlsym(s_hVulkanModule, "vkCreateInstance");
	if (vkCreateInstance == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateInstance\"." << Endl;
		return false;
	}

	vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)dlsym(s_hVulkanModule, "vkEnumerateInstanceLayerProperties");
	if (vkEnumerateInstanceLayerProperties == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkEnumerateInstanceLayerProperties\"." << Endl;
		return false;
	}

	vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)dlsym(s_hVulkanModule, "vkEnumerateInstanceExtensionProperties");
	if (vkEnumerateInstanceExtensionProperties == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkEnumerateInstanceExtensionProperties\"." << Endl;
		return false;
	}

	vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)dlsym(s_hVulkanModule, "vkEnumeratePhysicalDevices");
	if (vkEnumeratePhysicalDevices == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkEnumeratePhysicalDevices\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)dlsym(s_hVulkanModule, "vkGetPhysicalDeviceProperties");
	if (vkGetPhysicalDeviceProperties == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetPhysicalDeviceProperties\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)dlsym(s_hVulkanModule, "vkGetPhysicalDeviceQueueFamilyProperties");
	if (vkGetPhysicalDeviceQueueFamilyProperties == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetPhysicalDeviceQueueFamilyProperties\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)dlsym(s_hVulkanModule, "vkGetPhysicalDeviceMemoryProperties");
	if (vkGetPhysicalDeviceMemoryProperties == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetPhysicalDeviceMemoryProperties\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties)dlsym(s_hVulkanModule, "vkGetPhysicalDeviceFormatProperties");
	if (vkGetPhysicalDeviceFormatProperties == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetPhysicalDeviceFormatProperties\"." << Endl;
		return false;
	}

	vkCreateDevice = (PFN_vkCreateDevice)dlsym(s_hVulkanModule, "vkCreateDevice");
	if (vkCreateDevice == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateDevice\"." << Endl;
		return false;
	}

	vkGetDeviceQueue = (PFN_vkGetDeviceQueue)dlsym(s_hVulkanModule, "vkGetDeviceQueue");
	if (vkGetDeviceQueue == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetDeviceQueue\"." << Endl;
		return false;
	}

	vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(s_hVulkanModule, "vkGetInstanceProcAddr");
	if (vkGetInstanceProcAddr == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetInstanceProcAddr\"." << Endl;
		return false;
	}

	vkCreateCommandPool = (PFN_vkCreateCommandPool)dlsym(s_hVulkanModule, "vkCreateCommandPool");
	if (vkCreateCommandPool == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateCommandPool\"." << Endl;
		return false;
	}

	vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)dlsym(s_hVulkanModule, "vkAllocateCommandBuffers");
	if (vkAllocateCommandBuffers == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkAllocateCommandBuffers\"." << Endl;
		return false;
	}

	vkCreateShaderModule = (PFN_vkCreateShaderModule)dlsym(s_hVulkanModule, "vkCreateShaderModule");
	if (vkCreateShaderModule == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateShaderModule\"." << Endl;
		return false;
	}

	vkCreateFence = (PFN_vkCreateFence)dlsym(s_hVulkanModule, "vkCreateFence");
	if (vkCreateFence == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateFence\"." << Endl;
		return false;
	}

	vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)dlsym(s_hVulkanModule, "vkBeginCommandBuffer");
	if (vkBeginCommandBuffer == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkBeginCommandBuffer\"." << Endl;
		return false;
	}

	vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)dlsym(s_hVulkanModule, "vkCmdPipelineBarrier");
	if (vkCmdPipelineBarrier == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdPipelineBarrier\"." << Endl;
		return false;
	}

	vkEndCommandBuffer = (PFN_vkEndCommandBuffer)dlsym(s_hVulkanModule, "vkEndCommandBuffer");
	if (vkEndCommandBuffer == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkEndCommandBuffer\"." << Endl;
		return false;
	}

	vkQueueSubmit = (PFN_vkQueueSubmit)dlsym(s_hVulkanModule, "vkQueueSubmit");
	if (vkQueueSubmit == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkQueueSubmit\"." << Endl;
		return false;
	}

	vkWaitForFences = (PFN_vkWaitForFences)dlsym(s_hVulkanModule, "vkWaitForFences");
	if (vkWaitForFences == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkWaitForFences\"." << Endl;
		return false;
	}

	vkResetFences = (PFN_vkResetFences)dlsym(s_hVulkanModule, "vkResetFences");
	if (vkResetFences == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkResetFences\"." << Endl;
		return false;
	}

	vkResetCommandBuffer = (PFN_vkResetCommandBuffer)dlsym(s_hVulkanModule, "vkResetCommandBuffer");
	if (vkResetCommandBuffer == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkResetCommandBuffer\"." << Endl;
		return false;
	}

	vkCreateImageView = (PFN_vkCreateImageView)dlsym(s_hVulkanModule, "vkCreateImageView");
	if (vkCreateImageView == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateImageView\"." << Endl;
		return false;
	}

	vkCreateBuffer = (PFN_vkCreateBuffer)dlsym(s_hVulkanModule, "vkCreateBuffer");
	if (vkCreateBuffer == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateBuffer\"." << Endl;
		return false;
	}

	vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)dlsym(s_hVulkanModule, "vkGetBufferMemoryRequirements");
	if (vkGetBufferMemoryRequirements == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetBufferMemoryRequirements\"." << Endl;
		return false;
	}

	vkAllocateMemory = (PFN_vkAllocateMemory)dlsym(s_hVulkanModule, "vkAllocateMemory");
	if (vkAllocateMemory == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkAllocateMemory\"." << Endl;
		return false;
	}

	vkMapMemory = (PFN_vkMapMemory)dlsym(s_hVulkanModule, "vkMapMemory");
	if (vkMapMemory == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkMapMemory\"." << Endl;
		return false;
	}

	vkUnmapMemory = (PFN_vkUnmapMemory)dlsym(s_hVulkanModule, "vkUnmapMemory");
	if (vkUnmapMemory == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkUnmapMemory\"." << Endl;
		return false;
	}

	vkBindBufferMemory = (PFN_vkBindBufferMemory)dlsym(s_hVulkanModule, "vkBindBufferMemory");
	if (vkBindBufferMemory == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkBindBufferMemory\"." << Endl;
		return false;
	}

	vkCreateSemaphore = (PFN_vkCreateSemaphore)dlsym(s_hVulkanModule, "vkCreateSemaphore");
	if (vkCreateSemaphore == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateSemaphore\"." << Endl;
		return false;
	}

	vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)dlsym(s_hVulkanModule, "vkCmdBeginRenderPass");
	if (vkCmdBeginRenderPass == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdBeginRenderPass\"." << Endl;
		return false;
	}

	vkDestroySemaphore = (PFN_vkDestroySemaphore)dlsym(s_hVulkanModule, "vkDestroySemaphore");
	if (vkDestroySemaphore == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroySemaphore\"." << Endl;
		return false;
	}

	vkDestroyFence = (PFN_vkDestroyFence)dlsym(s_hVulkanModule, "vkDestroyFence");
	if (vkDestroyFence == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroyFence\"." << Endl;
		return false;
	}

	vkCreateImage = (PFN_vkCreateImage)dlsym(s_hVulkanModule, "vkCreateImage");
	if (vkCreateImage == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateImage\"." << Endl;
		return false;
	}

	vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)dlsym(s_hVulkanModule, "vkGetImageMemoryRequirements");
	if (vkGetImageMemoryRequirements == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetImageMemoryRequirements\"." << Endl;
		return false;
	}

	vkBindImageMemory = (PFN_vkBindImageMemory)dlsym(s_hVulkanModule, "vkBindImageMemory");
	if (vkBindImageMemory == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkBindImageMemory\"." << Endl;
		return false;
	}

	vkCreateRenderPass = (PFN_vkCreateRenderPass)dlsym(s_hVulkanModule, "vkCreateRenderPass");
	if (vkCreateRenderPass == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateRenderPass\"." << Endl;
		return false;
	}

	vkCreateFramebuffer = (PFN_vkCreateFramebuffer)dlsym(s_hVulkanModule, "vkCreateFramebuffer");
	if (vkCreateFramebuffer == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateFramebuffer\"." << Endl;
		return false;
	}

	vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass)dlsym(s_hVulkanModule, "vkCmdEndRenderPass");
	if (vkCmdEndRenderPass == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdEndRenderPass\"." << Endl;
		return false;
	}

	vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)dlsym(s_hVulkanModule, "vkCmdBindVertexBuffers");
	if (vkCmdBindVertexBuffers == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdBindVertexBuffers\"." << Endl;
		return false;
	}

	vkCmdDraw = (PFN_vkCmdDraw)dlsym(s_hVulkanModule, "vkCmdDraw");
	if (vkCmdDraw == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdDraw\"." << Endl;
		return false;
	}

	vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)dlsym(s_hVulkanModule, "vkCmdBindIndexBuffer");
	if (vkCmdBindIndexBuffer == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdBindIndexBuffer\"." << Endl;
		return false;
	}

	vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)dlsym(s_hVulkanModule, "vkCmdDrawIndexed");
	if (vkCmdDrawIndexed == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdDrawIndexed\"." << Endl;
		return false;
	}

	vkCmdBindPipeline = (PFN_vkCmdBindPipeline)dlsym(s_hVulkanModule, "vkCmdBindPipeline");
	if (vkCmdBindPipeline == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdBindPipeline\"." << Endl;
		return false;
	}

	vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout)dlsym(s_hVulkanModule, "vkCreatePipelineLayout");
	if (vkCreatePipelineLayout == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreatePipelineLayout\"." << Endl;
		return false;
	}

	vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)dlsym(s_hVulkanModule, "vkCreateGraphicsPipelines");
	if (vkCreateGraphicsPipelines == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateGraphicsPipelines\"." << Endl;
		return false;
	}

	vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)dlsym(s_hVulkanModule, "vkCreateDescriptorSetLayout");
	if (vkCreateDescriptorSetLayout == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateDescriptorSetLayout\"." << Endl;
		return false;
	}

	vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool)dlsym(s_hVulkanModule, "vkCreateDescriptorPool");
	if (vkCreateDescriptorPool == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateDescriptorPool\"." << Endl;
		return false;
	}

	vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)dlsym(s_hVulkanModule, "vkAllocateDescriptorSets");
	if (vkAllocateDescriptorSets == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkAllocateDescriptorSets\"." << Endl;
		return false;
	}

	vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)dlsym(s_hVulkanModule, "vkUpdateDescriptorSets");
	if (vkUpdateDescriptorSets == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkUpdateDescriptorSets\"." << Endl;
		return false;
	}

	vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)dlsym(s_hVulkanModule, "vkCmdBindDescriptorSets");
	if (vkCmdBindDescriptorSets == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdBindDescriptorSets\"." << Endl;
		return false;
	}

	vkDestroyPipeline = (PFN_vkDestroyPipeline)dlsym(s_hVulkanModule, "vkDestroyPipeline");
	if (vkDestroyPipeline == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroyPipeline\"." << Endl;
		return false;
	}

	vkCreateEvent = (PFN_vkCreateEvent)dlsym(s_hVulkanModule, "vkCreateEvent");
	if (vkCreateEvent == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateEvent\"." << Endl;
		return false;
	}

	vkDestroyEvent = (PFN_vkDestroyEvent)dlsym(s_hVulkanModule, "vkDestroyEvent");
	if (vkDestroyEvent == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroyEvent\"." << Endl;
		return false;
	}

	vkCmdSetEvent = (PFN_vkCmdSetEvent)dlsym(s_hVulkanModule, "vkCmdSetEvent");
	if (vkCmdSetEvent == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdSetEvent\"." << Endl;
		return false;
	}

	vkGetEventStatus = (PFN_vkGetEventStatus)dlsym(s_hVulkanModule, "vkGetEventStatus");
	if (vkGetEventStatus == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetEventStatus\"." << Endl;
		return false;
	}

	vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands)dlsym(s_hVulkanModule, "vkCmdExecuteCommands");
	if (vkCmdExecuteCommands == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdExecuteCommands\"." << Endl;
		return false;
	}

	vkResetDescriptorPool = (PFN_vkResetDescriptorPool)dlsym(s_hVulkanModule, "vkResetDescriptorPool");
	if (vkResetDescriptorPool == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkResetDescriptorPool\"." << Endl;
		return false;
	}

	vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers)dlsym(s_hVulkanModule, "vkFreeCommandBuffers");
	if (vkFreeCommandBuffers == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkFreeCommandBuffers\"." << Endl;
		return false;
	}

	vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle)dlsym(s_hVulkanModule, "vkDeviceWaitIdle");
	if (vkDeviceWaitIdle == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDeviceWaitIdle\"." << Endl;
		return false;
	}

	vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)dlsym(s_hVulkanModule, "vkFlushMappedMemoryRanges");
	if (vkFlushMappedMemoryRanges == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkFlushMappedMemoryRanges\"." << Endl;
		return false;
	}

	vkCreateSampler = (PFN_vkCreateSampler)dlsym(s_hVulkanModule, "vkCreateSampler");
	if (vkCreateSampler == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateSampler\"." << Endl;
		return false;
	}

	vkQueueWaitIdle = (PFN_vkQueueWaitIdle)dlsym(s_hVulkanModule, "vkQueueWaitIdle");
	if (vkQueueWaitIdle == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkQueueWaitIdle\"." << Endl;
		return false;
	}

	vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)dlsym(s_hVulkanModule, "vkCmdCopyBufferToImage");
	if (vkCmdCopyBufferToImage == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdCopyBufferToImage\"." << Endl;
		return false;
	}

	vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)dlsym(s_hVulkanModule, "vkEnumerateDeviceExtensionProperties");
	if (vkEnumerateDeviceExtensionProperties == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkEnumerateDeviceExtensionProperties\"." << Endl;
		return false;
	}

	vkCmdSetViewport = (PFN_vkCmdSetViewport)dlsym(s_hVulkanModule, "vkCmdSetViewport");
	if (vkCmdSetViewport == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdSetViewport\"." << Endl;
		return false;
	}

	vkFreeMemory = (PFN_vkFreeMemory)dlsym(s_hVulkanModule, "vkFreeMemory");
	if (vkFreeMemory == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkFreeMemory\"." << Endl;
		return false;
	}

	vkDestroyBuffer = (PFN_vkDestroyBuffer)dlsym(s_hVulkanModule, "vkDestroyBuffer");
	if (vkDestroyBuffer == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroyBuffer\"." << Endl;
		return false;
	}

	vkCmdCopyImage = (PFN_vkCmdCopyImage)dlsym(s_hVulkanModule, "vkCmdCopyImage");
	if (vkCmdCopyImage == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdCopyImage\"." << Endl;
		return false;
	}

	vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout)dlsym(s_hVulkanModule, "vkGetImageSubresourceLayout");
	if (vkGetImageSubresourceLayout == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetImageSubresourceLayout\"." << Endl;
		return false;
	}

	vkDestroyImage = (PFN_vkDestroyImage)dlsym(s_hVulkanModule, "vkDestroyImage");
	if (vkDestroyImage == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroyImage\"." << Endl;
		return false;
	}

	vkCmdDispatch = (PFN_vkCmdDispatch)dlsym(s_hVulkanModule, "vkCmdDispatch");
	if (vkCmdDispatch == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdDispatch\"." << Endl;
		return false;
	}

	vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)dlsym(s_hVulkanModule, "vkInvalidateMappedMemoryRanges");
	if (vkInvalidateMappedMemoryRanges == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkInvalidateMappedMemoryRanges\"." << Endl;
		return false;
	}

	vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)dlsym(s_hVulkanModule, "vkCmdCopyBuffer");
	if (vkCmdCopyBuffer == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdCopyBuffer\"." << Endl;
		return false;
	}

	vkDestroyShaderModule = (PFN_vkDestroyShaderModule)dlsym(s_hVulkanModule, "vkDestroyShaderModule");
	if (vkDestroyShaderModule == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroyShaderModule\"." << Endl;
		return false;
	}

	vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)dlsym(s_hVulkanModule, "vkDestroyDescriptorSetLayout");
	if (vkDestroyDescriptorSetLayout == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroyDescriptorSetLayout\"." << Endl;
		return false;
	}

	vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)dlsym(s_hVulkanModule, "vkDestroyPipelineLayout");
	if (vkDestroyPipelineLayout == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroyPipelineLayout\"." << Endl;
		return false;
	}

	vkDestroySampler = (PFN_vkDestroySampler)dlsym(s_hVulkanModule, "vkDestroySampler");
	if (vkDestroySampler == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroySampler\"." << Endl;
		return false;
	}			

	vkDestroyCommandPool = (PFN_vkDestroyCommandPool)dlsym(s_hVulkanModule, "vkDestroyCommandPool");
	if (vkDestroyCommandPool == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroyCommandPool\"." << Endl;
		return false;
	}

	vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)dlsym(s_hVulkanModule, "vkDestroyDescriptorPool");
	if (vkDestroyDescriptorPool == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkDestroyDescriptorPool\"." << Endl;
		return false;
	}

	vkCmdClearColorImage = (PFN_vkCmdClearColorImage)dlsym(s_hVulkanModule, "vkCmdClearColorImage");
	if (vkCmdClearColorImage == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdClearColorImage\"." << Endl;
		return false;
	}

	vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage)dlsym(s_hVulkanModule, "vkCmdClearDepthStencilImage");
	if (vkCmdClearDepthStencilImage == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdClearDepthStencilImage\"." << Endl;
		return false;
	}
	
	vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference)dlsym(s_hVulkanModule, "vkCmdSetStencilReference");
	if (vkCmdSetStencilReference == nullptr)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCmdSetStencilReference\"." << Endl;
		return false;
	}

	return true;
}

bool initializeVulkanExtensions(VkInstance instance)
{
	vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
	if (vkGetPhysicalDeviceSurfaceSupportKHR == nullptr)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkGetPhysicalDeviceSurfaceSupportKHR\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
	if (vkGetPhysicalDeviceSurfaceFormatsKHR == nullptr)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkGetPhysicalDeviceSurfaceFormatsKHR\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR == nullptr)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkGetPhysicalDeviceSurfaceCapabilitiesKHR\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
	if (vkGetPhysicalDeviceSurfacePresentModesKHR == nullptr)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkGetPhysicalDeviceSurfacePresentModesKHR\"." << Endl;
		return false;
	}

	vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)vkGetInstanceProcAddr(instance, "vkCreateSwapchainKHR");
	if (vkCreateSwapchainKHR == nullptr)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkCreateSwapchainKHR\"." << Endl;
		return false;
	}

	vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)vkGetInstanceProcAddr(instance, "vkGetSwapchainImagesKHR");
	if (vkGetSwapchainImagesKHR == nullptr)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkGetSwapchainImagesKHR\"." << Endl;
		return false;
	}

	vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)vkGetInstanceProcAddr(instance, "vkAcquireNextImageKHR");
	if (vkAcquireNextImageKHR == nullptr)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkAcquireNextImageKHR\"." << Endl;
		return false;
	}

	vkQueuePresentKHR = (PFN_vkQueuePresentKHR)vkGetInstanceProcAddr(instance, "vkQueuePresentKHR");
	if (vkQueuePresentKHR == nullptr)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkQueuePresentKHR\"." << Endl;
		return false;
	}

	vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)vkGetInstanceProcAddr(instance, "vkDestroySwapchainKHR");
	if (vkDestroySwapchainKHR == nullptr)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkDestroySwapchainKHR\"." << Endl;
		return false;
	}

	vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateAndroidSurfaceKHR");
	if (vkCreateAndroidSurfaceKHR == nullptr)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkCreateAndroidSurfaceKHR\"." << Endl;
		return false;
	}

	//vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	//if (vkCreateDebugUtilsMessengerEXT == nullptr)
	//{
	//	log::error << L"Failed to resolve Vulkan entry point \"vkCreateDebugUtilsMessengerEXT\"." << Endl;
	//	return false;
	//}

	//vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
	//if (vkSetDebugUtilsObjectNameEXT == nullptr)
	//{
	//	log::error << L"Failed to resolve Vulkan entry point \"vkSetDebugUtilsObjectNameEXT\"." << Endl;
	//	return false;
	//}

	return true;
}

void finalizeVulkanApi()
{
}

	}
}
