#pragma once

#define VK_USE_PLATFORM_IOS_MVK
#include <vulkan/vulkan.h>

namespace traktor
{
	namespace render
	{

bool initializeVulkanApi();

bool initializeVulkanExtensions(VkInstance instance);

void finalizeVulkanApi();

	}
}

