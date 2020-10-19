#pragma once

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
		//VmaAllocator allocator,
		//VmaAllocation allocation,
		//VkBuffer vertexBuffer,
		const VkVertexInputBindingDescription& vertexBindingDescription,
		const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
		uint32_t hash
	);

	virtual VkBuffer getVkBuffer() const = 0;

	const VkVertexInputBindingDescription& getVkVertexInputBindingDescription() const { return m_vertexBindingDescription; }

	const AlignedVector< VkVertexInputAttributeDescription >& getVkVertexInputAttributeDescriptions() const { return m_vertexAttributeDescriptions; }

	uint32_t getHash() const { return m_hash; }

protected:
	//VmaAllocator m_allocator;
	//VmaAllocation m_allocation;
	//VkBuffer m_vertexBuffer;

	VkVertexInputBindingDescription m_vertexBindingDescription;
	AlignedVector< VkVertexInputAttributeDescription > m_vertexAttributeDescriptions;
	uint32_t m_hash;

	//Buffer m_buffer;
};

	}
}

