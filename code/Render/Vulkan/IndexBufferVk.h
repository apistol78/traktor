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
#endif
#include <vk_mem_alloc.h>

#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class IndexBufferVk : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferVk(
		IndexType indexType,
		uint32_t bufferSize,
		VmaAllocator allocator,
		VmaAllocation allocation,
		VkBuffer indexBuffer
	);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	VkBuffer getVkBuffer() const { return m_indexBuffer; }

private:
	VmaAllocator m_allocator;
	VmaAllocation m_allocation;
	VkBuffer m_indexBuffer;
	bool m_locked;
};

	}
}

