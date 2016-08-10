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
extern PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

bool initializeVulkanApi();

void finalizeVulkanApi();

	}
}

#endif	// traktor_render_ApiLoader_H
