/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{

uint32_t getMemoryTypeIndex(VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags memoryFlags, const VkMemoryRequirements& memoryRequirements)
{
	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	uint32_t memoryTypeBits = memoryRequirements.memoryTypeBits;
	for (uint32_t i = 0; i < 32; ++i)
	{
		VkMemoryType memoryType = memoryProperties.memoryTypes[i];
		if (memoryTypeBits & 1)
		{
			if ((memoryType.propertyFlags & memoryFlags) == memoryFlags)
				return i;
		}
		memoryTypeBits = memoryTypeBits >> 1;
	}
	return 0; 
}

bool changeImageLayout(VkDevice device, VkQueue presentQueue, VkCommandBuffer setupCmdBuffer, VkImage image, VkAccessFlags dstAccessMask, VkImageLayout newLayout)
{
	VkFence submitFence;
 
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(device, &fenceCreateInfo, nullptr, &submitFence);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(setupCmdBuffer, &beginInfo);

	VkImageMemoryBarrier layoutTransitionBarrier = {};
	layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutTransitionBarrier.srcAccessMask = 0;
	layoutTransitionBarrier.dstAccessMask = dstAccessMask;
	layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	layoutTransitionBarrier.newLayout = newLayout;
	layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.image = image;
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
		return false;
 
	vkWaitForFences(device, 1, &submitFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &submitFence);
	vkResetCommandBuffer(setupCmdBuffer, 0);
	vkDestroyFence(device, submitFence, nullptr);
	return true;
}

	}
}
