#ifndef traktor_render_ContextVk_H
#define traktor_render_ContextVk_H

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#endif

#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

/*! \brief
 * \ingroup Vulkan
 */
class ContextVk : public Object
{
public:
	ContextVk(VkInstance vkInstance, VkSurfaceKHR vkSurface, VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice, VkQueue vkPresentQueue);

	VkInstance getVkInstance() { return m_vkInstance; }

	VkSurfaceKHR getVkSurfaceKHR() { return m_vkSurface; }

	VkPhysicalDevice getVkPhysicalDevice() { return m_vkPhysicalDevice; }

	VkDevice getVkDevice() { return m_vkDevice; }

	VkQueue getVkQueue() { return m_vkPresentQueue; }

private:
	VkInstance m_vkInstance;
	VkSurfaceKHR m_vkSurface;
	VkPhysicalDevice m_vkPhysicalDevice;
	VkDevice m_vkDevice;
	VkQueue m_vkPresentQueue;
};

	}
}

#endif	// traktor_render_ContextVk_H
