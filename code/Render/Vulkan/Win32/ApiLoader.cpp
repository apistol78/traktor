/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Render/Vulkan/Win32/ApiLoader.h"

#define T_DEFINE_VK(fn) \
	PFN_##fn fn = nullptr;

#define T_RESOLVE_VK(fn) \
	fn = (PFN_##fn)GetProcAddress(s_hVulkanModule, #fn); \
	if (!fn) { \
		log::error << L"Failed to resolve Vulkan entry point \"" #fn L"\"." << Endl; \
		return false; \
	}

#define T_RESOLVE_VK_EXT(fn) \
	*(void**)&fn = vkGetInstanceProcAddr(instance, #fn); \
	if (!fn) \
	{ \
		log::error << L"Failed to resolve Vulkan extension entry point \"" #fn L"\"." << Endl; \
		return false; \
	}

T_DEFINE_VK(vkCreateInstance);
T_DEFINE_VK(vkEnumerateInstanceLayerProperties);
T_DEFINE_VK(vkEnumerateInstanceExtensionProperties);
T_DEFINE_VK(vkEnumeratePhysicalDevices);
T_DEFINE_VK(vkGetPhysicalDeviceProperties);
T_DEFINE_VK(vkGetPhysicalDeviceQueueFamilyProperties);
T_DEFINE_VK(vkGetPhysicalDeviceMemoryProperties);
T_DEFINE_VK(vkGetPhysicalDeviceFormatProperties);
T_DEFINE_VK(vkCreateDevice);
T_DEFINE_VK(vkGetDeviceQueue);
T_DEFINE_VK(vkGetDeviceProcAddr);
T_DEFINE_VK(vkGetInstanceProcAddr);
T_DEFINE_VK(vkCreateCommandPool);
T_DEFINE_VK(vkAllocateCommandBuffers);
T_DEFINE_VK(vkCreateShaderModule);
T_DEFINE_VK(vkCreateFence);
T_DEFINE_VK(vkBeginCommandBuffer);
T_DEFINE_VK(vkCmdPipelineBarrier);
T_DEFINE_VK(vkEndCommandBuffer);
T_DEFINE_VK(vkQueueSubmit);
T_DEFINE_VK(vkWaitForFences);
T_DEFINE_VK(vkResetFences);
T_DEFINE_VK(vkResetCommandBuffer);
T_DEFINE_VK(vkCreateImageView);
T_DEFINE_VK(vkDestroyImageView);
T_DEFINE_VK(vkCreateBuffer);
T_DEFINE_VK(vkGetBufferMemoryRequirements);
T_DEFINE_VK(vkAllocateMemory);
T_DEFINE_VK(vkMapMemory);
T_DEFINE_VK(vkUnmapMemory);
T_DEFINE_VK(vkBindBufferMemory);
T_DEFINE_VK(vkCreateSemaphore);
T_DEFINE_VK(vkCmdBeginRenderPass);
T_DEFINE_VK(vkDestroySemaphore);
T_DEFINE_VK(vkDestroyFence);
T_DEFINE_VK(vkCreateImage);
T_DEFINE_VK(vkGetImageMemoryRequirements);
T_DEFINE_VK(vkBindImageMemory);
T_DEFINE_VK(vkCreateRenderPass);
T_DEFINE_VK(vkCreateFramebuffer);
T_DEFINE_VK(vkCmdEndRenderPass);
T_DEFINE_VK(vkCmdBindVertexBuffers);
T_DEFINE_VK(vkCmdDraw);
T_DEFINE_VK(vkCmdBindIndexBuffer);
T_DEFINE_VK(vkCmdDrawIndexed);
T_DEFINE_VK(vkCmdBindPipeline);
T_DEFINE_VK(vkCreatePipelineLayout);
T_DEFINE_VK(vkCreateGraphicsPipelines);
T_DEFINE_VK(vkCreateComputePipelines);
T_DEFINE_VK(vkCreateDescriptorSetLayout);
T_DEFINE_VK(vkCreateDescriptorPool);
T_DEFINE_VK(vkAllocateDescriptorSets);
T_DEFINE_VK(vkUpdateDescriptorSets);
T_DEFINE_VK(vkCmdBindDescriptorSets);
T_DEFINE_VK(vkDestroyPipeline);
T_DEFINE_VK(vkCreateEvent);
T_DEFINE_VK(vkDestroyEvent);
T_DEFINE_VK(vkCmdSetEvent);
T_DEFINE_VK(vkGetEventStatus);
T_DEFINE_VK(vkCmdExecuteCommands);
T_DEFINE_VK(vkResetDescriptorPool);
T_DEFINE_VK(vkFreeCommandBuffers);
T_DEFINE_VK(vkDeviceWaitIdle);
T_DEFINE_VK(vkFlushMappedMemoryRanges);
T_DEFINE_VK(vkCreateSampler);
T_DEFINE_VK(vkQueueWaitIdle);
T_DEFINE_VK(vkCmdCopyBufferToImage);
T_DEFINE_VK(vkEnumerateDeviceExtensionProperties);
T_DEFINE_VK(vkCmdSetViewport);
T_DEFINE_VK(vkFreeMemory);
T_DEFINE_VK(vkDestroyBuffer);
T_DEFINE_VK(vkCmdCopyImage);
T_DEFINE_VK(vkGetImageSubresourceLayout);
T_DEFINE_VK(vkDestroyImage);
T_DEFINE_VK(vkCmdDispatch);
T_DEFINE_VK(vkInvalidateMappedMemoryRanges);
T_DEFINE_VK(vkCmdCopyBuffer);
T_DEFINE_VK(vkDestroyShaderModule);
T_DEFINE_VK(vkDestroyDescriptorSetLayout);
T_DEFINE_VK(vkDestroyPipelineLayout);
T_DEFINE_VK(vkDestroySampler);
T_DEFINE_VK(vkDestroyCommandPool);
T_DEFINE_VK(vkDestroyDescriptorPool);
T_DEFINE_VK(vkCmdClearColorImage);
T_DEFINE_VK(vkCmdClearDepthStencilImage);
T_DEFINE_VK(vkCmdSetStencilReference);
T_DEFINE_VK(vkCreateQueryPool);
T_DEFINE_VK(vkDestroyQueryPool);
T_DEFINE_VK(vkCmdResetQueryPool);
T_DEFINE_VK(vkCmdWriteTimestamp);
T_DEFINE_VK(vkGetQueryPoolResults);
T_DEFINE_VK(vkCreatePipelineCache);
T_DEFINE_VK(vkGetPipelineCacheData);
T_DEFINE_VK(vkFreeDescriptorSets);
T_DEFINE_VK(vkCmdDrawIndirect);
T_DEFINE_VK(vkCmdDrawIndexedIndirect);
T_DEFINE_VK(vkCmdPushConstants);

T_DEFINE_VK(vkCreateWin32SurfaceKHR);
T_DEFINE_VK(vkDestroySurfaceKHR);
T_DEFINE_VK(vkGetPhysicalDeviceSurfaceSupportKHR);
T_DEFINE_VK(vkGetPhysicalDeviceSurfaceFormatsKHR);
T_DEFINE_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
T_DEFINE_VK(vkGetPhysicalDeviceSurfacePresentModesKHR);
T_DEFINE_VK(vkCreateSwapchainKHR);
T_DEFINE_VK(vkGetSwapchainImagesKHR);
T_DEFINE_VK(vkAcquireNextImageKHR);
T_DEFINE_VK(vkQueuePresentKHR);
T_DEFINE_VK(vkDestroySwapchainKHR);
T_DEFINE_VK(vkCreateDebugUtilsMessengerEXT);
T_DEFINE_VK(vkSetDebugUtilsObjectNameEXT);
T_DEFINE_VK(vkCmdBeginDebugUtilsLabelEXT);
T_DEFINE_VK(vkCmdEndDebugUtilsLabelEXT);
T_DEFINE_VK(vkGetBufferMemoryRequirements2KHR);
T_DEFINE_VK(vkGetImageMemoryRequirements2KHR);

namespace traktor
{
	namespace render
	{
		namespace
		{

HMODULE s_hVulkanModule = nullptr;

		}

bool initializeVulkanApi()
{
	s_hVulkanModule = LoadLibrary(L"vulkan-1.dll");
	if (s_hVulkanModule == nullptr)
	{
		log::error << L"Failed to load Vulkan library." << Endl;
		return false;
	}

	T_RESOLVE_VK(vkCreateInstance);
	T_RESOLVE_VK(vkEnumerateInstanceLayerProperties);
	T_RESOLVE_VK(vkEnumerateInstanceExtensionProperties);
	T_RESOLVE_VK(vkEnumeratePhysicalDevices);
	T_RESOLVE_VK(vkGetPhysicalDeviceProperties);
	T_RESOLVE_VK(vkGetPhysicalDeviceQueueFamilyProperties);
	T_RESOLVE_VK(vkGetPhysicalDeviceMemoryProperties);
	T_RESOLVE_VK(vkGetPhysicalDeviceFormatProperties);
	T_RESOLVE_VK(vkCreateDevice);
	T_RESOLVE_VK(vkGetDeviceQueue);
	T_RESOLVE_VK(vkGetDeviceProcAddr);
	T_RESOLVE_VK(vkGetInstanceProcAddr);
	T_RESOLVE_VK(vkCreateCommandPool);
	T_RESOLVE_VK(vkAllocateCommandBuffers);
	T_RESOLVE_VK(vkCreateShaderModule);
	T_RESOLVE_VK(vkCreateFence);
	T_RESOLVE_VK(vkBeginCommandBuffer);
	T_RESOLVE_VK(vkCmdPipelineBarrier);
	T_RESOLVE_VK(vkEndCommandBuffer);
	T_RESOLVE_VK(vkQueueSubmit);
	T_RESOLVE_VK(vkWaitForFences);
	T_RESOLVE_VK(vkResetFences);
	T_RESOLVE_VK(vkResetCommandBuffer);
	T_RESOLVE_VK(vkCreateImageView);
	T_RESOLVE_VK(vkDestroyImageView);
	T_RESOLVE_VK(vkCreateBuffer);
	T_RESOLVE_VK(vkGetBufferMemoryRequirements);
	T_RESOLVE_VK(vkAllocateMemory);
	T_RESOLVE_VK(vkMapMemory);
	T_RESOLVE_VK(vkUnmapMemory);
	T_RESOLVE_VK(vkBindBufferMemory);
	T_RESOLVE_VK(vkCreateSemaphore);
	T_RESOLVE_VK(vkCmdBeginRenderPass);
	T_RESOLVE_VK(vkDestroySemaphore);
	T_RESOLVE_VK(vkDestroyFence);
	T_RESOLVE_VK(vkCreateImage);
	T_RESOLVE_VK(vkGetImageMemoryRequirements);
	T_RESOLVE_VK(vkBindImageMemory);
	T_RESOLVE_VK(vkCreateRenderPass);
	T_RESOLVE_VK(vkCreateFramebuffer);
	T_RESOLVE_VK(vkCmdEndRenderPass);
	T_RESOLVE_VK(vkCmdBindVertexBuffers);
	T_RESOLVE_VK(vkCmdDraw);
	T_RESOLVE_VK(vkCmdBindIndexBuffer);
	T_RESOLVE_VK(vkCmdDrawIndexed);
	T_RESOLVE_VK(vkCmdBindPipeline);
	T_RESOLVE_VK(vkCreatePipelineLayout);
	T_RESOLVE_VK(vkCreateGraphicsPipelines);
	T_RESOLVE_VK(vkCreateComputePipelines);
	T_RESOLVE_VK(vkCreateDescriptorSetLayout);
	T_RESOLVE_VK(vkCreateDescriptorPool);
	T_RESOLVE_VK(vkAllocateDescriptorSets);
	T_RESOLVE_VK(vkUpdateDescriptorSets);
	T_RESOLVE_VK(vkCmdBindDescriptorSets);
	T_RESOLVE_VK(vkDestroyPipeline);
	T_RESOLVE_VK(vkCreateEvent);
	T_RESOLVE_VK(vkDestroyEvent);
	T_RESOLVE_VK(vkCmdSetEvent);
	T_RESOLVE_VK(vkGetEventStatus);
	T_RESOLVE_VK(vkCmdExecuteCommands);
	T_RESOLVE_VK(vkResetDescriptorPool);
	T_RESOLVE_VK(vkFreeCommandBuffers);
	T_RESOLVE_VK(vkDeviceWaitIdle);
	T_RESOLVE_VK(vkFlushMappedMemoryRanges);
	T_RESOLVE_VK(vkCreateSampler);
	T_RESOLVE_VK(vkQueueWaitIdle);
	T_RESOLVE_VK(vkCmdCopyBufferToImage);
	T_RESOLVE_VK(vkEnumerateDeviceExtensionProperties);
	T_RESOLVE_VK(vkCmdSetViewport);
	T_RESOLVE_VK(vkFreeMemory);
	T_RESOLVE_VK(vkDestroyBuffer);
	T_RESOLVE_VK(vkCmdCopyImage);
	T_RESOLVE_VK(vkGetImageSubresourceLayout);
	T_RESOLVE_VK(vkDestroyImage);
	T_RESOLVE_VK(vkCmdDispatch);
	T_RESOLVE_VK(vkInvalidateMappedMemoryRanges);
	T_RESOLVE_VK(vkCmdCopyBuffer);
	T_RESOLVE_VK(vkDestroyShaderModule);
	T_RESOLVE_VK(vkDestroyDescriptorSetLayout);
	T_RESOLVE_VK(vkDestroyPipelineLayout);
	T_RESOLVE_VK(vkDestroySampler);
	T_RESOLVE_VK(vkDestroyCommandPool);
	T_RESOLVE_VK(vkDestroyDescriptorPool);
	T_RESOLVE_VK(vkCmdClearColorImage);
	T_RESOLVE_VK(vkCmdClearDepthStencilImage);
	T_RESOLVE_VK(vkCmdSetStencilReference);
	T_RESOLVE_VK(vkCreateQueryPool);
	T_RESOLVE_VK(vkDestroyQueryPool);
	T_RESOLVE_VK(vkCmdResetQueryPool);
	T_RESOLVE_VK(vkCmdWriteTimestamp);
	T_RESOLVE_VK(vkGetQueryPoolResults);
	T_RESOLVE_VK(vkCreatePipelineCache);
	T_RESOLVE_VK(vkGetPipelineCacheData);
	T_RESOLVE_VK(vkFreeDescriptorSets);
	T_RESOLVE_VK(vkCmdDrawIndirect);
	T_RESOLVE_VK(vkCmdDrawIndexedIndirect);
	T_RESOLVE_VK(vkCmdPushConstants);
	return true;
}

bool initializeVulkanExtensions(VkInstance instance)
{
	T_RESOLVE_VK_EXT(vkCreateWin32SurfaceKHR);
	T_RESOLVE_VK_EXT(vkDestroySurfaceKHR);
	T_RESOLVE_VK_EXT(vkGetPhysicalDeviceSurfaceSupportKHR);
	T_RESOLVE_VK_EXT(vkGetPhysicalDeviceSurfaceFormatsKHR);
	T_RESOLVE_VK_EXT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	T_RESOLVE_VK_EXT(vkGetPhysicalDeviceSurfacePresentModesKHR);
	T_RESOLVE_VK_EXT(vkCreateSwapchainKHR);
	T_RESOLVE_VK_EXT(vkGetSwapchainImagesKHR);
	T_RESOLVE_VK_EXT(vkAcquireNextImageKHR);
	T_RESOLVE_VK_EXT(vkQueuePresentKHR);
	T_RESOLVE_VK_EXT(vkDestroySwapchainKHR);
	T_RESOLVE_VK_EXT(vkCreateDebugUtilsMessengerEXT);
	T_RESOLVE_VK_EXT(vkSetDebugUtilsObjectNameEXT);
	T_RESOLVE_VK_EXT(vkCmdBeginDebugUtilsLabelEXT);
	T_RESOLVE_VK_EXT(vkCmdEndDebugUtilsLabelEXT);
	T_RESOLVE_VK_EXT(vkGetBufferMemoryRequirements2KHR);
	T_RESOLVE_VK_EXT(vkGetImageMemoryRequirements2KHR);	
	return true;
}

void finalizeVulkanApi()
{
}

	}
}
