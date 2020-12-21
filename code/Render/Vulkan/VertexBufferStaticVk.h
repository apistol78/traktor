#pragma once

#include "Render/Vulkan/VertexBufferVk.h"

namespace traktor
{
	namespace render
	{

class Buffer;
class CommandBufferPool;
class Context;
class Queue;

/*!
 * \ingroup Vulkan
 */
class VertexBufferStaticVk : public VertexBufferVk
{
	T_RTTI_CLASS;

public:
	VertexBufferStaticVk(
		Context* context,
		Queue* graphicsQueue,
		CommandBufferPool* graphicsCommandPool,
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
	Ref< Context > m_context;
	Ref< Queue > m_graphicsQueue;
	Ref< CommandBufferPool > m_graphicsCommandPool;
	Ref< Buffer > m_stageBuffer;
	Ref< Buffer > m_deviceBuffer;
};

	}
}

