#pragma once

#include "Render/Vulkan/Buffer.h"
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
		VmaAllocator allocator,
		uint32_t bufferSize,
		const VkVertexInputBindingDescription& vertexBindingDescription,
		const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
		uint32_t hash
	);

	bool create();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) override final;

	virtual void unlock() override final;

	virtual VkBuffer getVkBuffer() const override final;

private:
	VkDevice m_logicalDevice;
	Ref< Queue > m_graphicsQueue;
	Ref< CommandBufferPool > m_graphicsCommandPool;
	VmaAllocator m_allocator;
	Buffer m_stageBuffer;
	Buffer m_deviceBuffer;
};

	}
}

