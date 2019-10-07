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

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace render
	{
	
class UniformBufferPoolVk : public Object
{
	T_RTTI_CLASS;

public:
	enum { MaxPendingFrames = 4 };

	UniformBufferPoolVk(VmaAllocator allocator);

	bool acquire(
		uint32_t size,
		VkBuffer& inoutBuffer,
		VmaAllocation& inoutAllocation
	);

	void collect();

private:
	struct BufferChain
	{
		uint32_t size;
		VkBuffer buffer;
		VmaAllocation allocation;
	};

	VmaAllocator m_allocator;
	AlignedVector< BufferChain > m_free;
	AlignedVector< BufferChain > m_released[MaxPendingFrames];
	uint32_t m_counter;
};

	}
}
