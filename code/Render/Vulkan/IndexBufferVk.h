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
	IndexBufferVk(IndexType indexType, uint32_t bufferSize, VkDevice device, VkBuffer indexBuffer, VkDeviceMemory indexBufferMemory);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	VkBuffer getVkBuffer() const { return m_indexBuffer; }

private:
	VkDevice m_device;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;
};

	}
}

