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

#include "Core/Containers/AlignedVector.h"
#include "Render/VertexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class VertexBufferVk : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	VertexBufferVk(
		uint32_t bufferSize,
		VkDevice device,
		VkBuffer vertexBuffer,
		VkDeviceMemory vertexBufferMemory,
		const VkVertexInputBindingDescription& vertexBindingDescription,
		const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
		uint32_t hash
	);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) override final;

	virtual void unlock() override final;

	VkBuffer getVkBuffer() const { return m_vertexBuffer; }

	const VkVertexInputBindingDescription& getVkVertexInputBindingDescription() const { return m_vertexBindingDescription; }

	const AlignedVector< VkVertexInputAttributeDescription >& getVkVertexInputAttributeDescriptions() const { return m_vertexAttributeDescriptions; }

	uint32_t getHash() const { return m_hash; }

private:
	VkDevice m_device;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkVertexInputBindingDescription m_vertexBindingDescription;
	AlignedVector< VkVertexInputAttributeDescription > m_vertexAttributeDescriptions;
	uint32_t m_hash;
};

	}
}

