#pragma once

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__LINUX__)
#	define VK_USE_PLATFORM_LINUX_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__APPLE__)
#	define VK_USE_PLATFORM_MACOS_MVK
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#endif
#include <vk_mem_alloc.h>

#include "Render/StructBuffer.h"

namespace traktor
{
	namespace render
	{

class StructBufferVk : public StructBuffer
{
	T_RTTI_CLASS;

public:
	StructBufferVk(
		uint32_t bufferSize,
		VmaAllocator allocator,
		VmaAllocation allocation,
		VkBuffer storageBuffer
	);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t structOffset, uint32_t structCount) override final;

	virtual void unlock() override final;

	VkBuffer getVkBuffer() const { return m_storageBuffer; }

private:
	VmaAllocator m_allocator;
	VmaAllocation m_allocation;
	VkBuffer m_storageBuffer;
	bool m_locked;
};

	}
}
