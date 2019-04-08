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

#include "Render/StructBuffer.h"

namespace traktor
{
	namespace render
	{

class StructBufferVk : public StructBuffer
{
	T_RTTI_CLASS;

public:
	StructBufferVk(uint32_t bufferSize, VkDevice device, VkBuffer storageBuffer, VkDeviceMemory storageBufferMemory);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t structOffset, uint32_t structCount) override final;

	virtual void unlock() override final;

private:
	VkDevice m_device;
	VkBuffer m_storageBuffer;
	VkDeviceMemory m_storageBufferMemory;
};

	}
}
