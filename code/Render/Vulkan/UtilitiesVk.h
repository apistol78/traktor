#pragma once

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__LINUX__)
#	define VK_USE_PLATFORM_LINUX_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#endif

#include "Core/Config.h"

namespace traktor
{
	namespace render
	{

uint32_t getMemoryTypeIndex(VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags memoryFlags, const VkMemoryRequirements& memoryRequirements);

bool changeImageLayout(VkDevice device, VkQueue presentQueue, VkCommandBuffer setupCmdBuffer, VkImage image, VkAccessFlags dstAccessMask, VkImageLayout newLayout);

const wchar_t* getHumanResult(VkResult result);

	}
}

