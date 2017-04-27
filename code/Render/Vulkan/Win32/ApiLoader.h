/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ApiLoader_H
#define traktor_render_ApiLoader_H

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan.h>

namespace traktor
{
	namespace render
	{

extern PFN_vkCreateInstance vkCreateInstance;
extern PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
extern PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
extern PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
extern PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
extern PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
extern PFN_vkCreateDevice vkCreateDevice;
extern PFN_vkGetDeviceQueue vkGetDeviceQueue;
extern PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
extern PFN_vkCreateCommandPool vkCreateCommandPool;
extern PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
extern PFN_vkCreateShaderModule vkCreateShaderModule;
extern PFN_vkCreateFence vkCreateFence;
extern PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
extern PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
extern PFN_vkEndCommandBuffer vkEndCommandBuffer;
extern PFN_vkQueueSubmit vkQueueSubmit;
extern PFN_vkWaitForFences vkWaitForFences;
extern PFN_vkResetFences vkResetFences;
extern PFN_vkResetCommandBuffer vkResetCommandBuffer;
extern PFN_vkCreateImageView vkCreateImageView;
extern PFN_vkCreateBuffer vkCreateBuffer;
extern PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
extern PFN_vkAllocateMemory vkAllocateMemory;
extern PFN_vkMapMemory vkMapMemory;
extern PFN_vkUnmapMemory vkUnmapMemory;
extern PFN_vkBindBufferMemory vkBindBufferMemory;
extern PFN_vkCreateSemaphore vkCreateSemaphore;
extern PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
extern PFN_vkDestroySemaphore vkDestroySemaphore;
extern PFN_vkDestroyFence vkDestroyFence;
extern PFN_vkCreateImage vkCreateImage;
extern PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
extern PFN_vkBindImageMemory vkBindImageMemory;
extern PFN_vkCreateRenderPass vkCreateRenderPass;
extern PFN_vkCreateFramebuffer vkCreateFramebuffer;
extern PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
extern PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
extern PFN_vkCmdDraw vkCmdDraw;
extern PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
extern PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
extern PFN_vkCmdBindPipeline vkCmdBindPipeline;
extern PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
extern PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
extern PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
extern PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
extern PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
extern PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
extern PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
extern PFN_vkDestroyPipeline vkDestroyPipeline;
extern PFN_vkCreateEvent vkCreateEvent;
extern PFN_vkDestroyEvent vkDestroyEvent;
extern PFN_vkCmdSetEvent vkCmdSetEvent;
extern PFN_vkGetEventStatus vkGetEventStatus;
extern PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
extern PFN_vkResetDescriptorPool vkResetDescriptorPool;
extern PFN_vkFreeCommandBuffers vkFreeCommandBuffers;

// Win32 Vulkan extensions.
extern PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
extern PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
extern PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
extern PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
extern PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
extern PFN_vkQueuePresentKHR vkQueuePresentKHR;

bool initializeVulkanApi();

bool initializeVulkanExtensions(VkInstance instance);

void finalizeVulkanApi();

	}
}

#endif	// traktor_render_ApiLoader_H
