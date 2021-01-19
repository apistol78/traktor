#pragma once

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__LINUX__)
#	define VK_USE_PLATFORM_XLIB_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__MAC__)
#	define VK_USE_PLATFORM_METAL_EXT
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__IOS__)
#	define VK_USE_PLATFORM_IOS_MVK
#	include <vulkan/vulkan.h>
#endif
#include <vk_mem_alloc.h>
