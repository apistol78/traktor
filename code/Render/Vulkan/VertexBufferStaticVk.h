#pragma once

#include "Render/Vulkan/VertexBufferVk.h"

namespace traktor
{
	namespace render
	{

class CommandBufferPool;
class Queue;

/*!
 * \ingroup Vulkan
 */
class VertexBufferStaticVk : public VertexBufferVk
{
	T_RTTI_CLASS;

public:
	VertexBufferStaticVk(
		VkDevice logicalDevice,
		Queue* graphicsQueue,
		CommandBufferPool* graphicsCommandPool,
		uint32_t bufferSize,
		VmaAllocator allocator,
		VmaAllocation allocation,
		VkBuffer vertexBuffer,
		const VkVertexInputBindingDescription& vertexBindingDescription,
		const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
		uint32_t hash
	);

	virtual void* lock() override final;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) override final;

	virtual void unlock() override final;

private:
	VkDevice m_logicalDevice;
	Ref< Queue > m_graphicsQueue;
	Ref< CommandBufferPool > m_graphicsCommandPool;
	VmaAllocation m_stagingBufferAllocation;
	VkBuffer m_stagingBuffer;
};

	}
}

