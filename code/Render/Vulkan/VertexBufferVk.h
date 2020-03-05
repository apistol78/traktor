#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/VertexBuffer.h"
#include "Render/Vulkan/ApiHeader.h"

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
		VmaAllocator allocator,
		VmaAllocation allocation,
		VkBuffer vertexBuffer,
		const VkVertexInputBindingDescription& vertexBindingDescription,
		const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
		uint32_t hash
	);

	virtual void destroy() override;

	VkBuffer getVkBuffer() const { return m_vertexBuffer; }

	const VkVertexInputBindingDescription& getVkVertexInputBindingDescription() const { return m_vertexBindingDescription; }

	const AlignedVector< VkVertexInputAttributeDescription >& getVkVertexInputAttributeDescriptions() const { return m_vertexAttributeDescriptions; }

	uint32_t getHash() const { return m_hash; }

protected:
	VmaAllocator m_allocator;
	VmaAllocation m_allocation;
	VkBuffer m_vertexBuffer;
	VkVertexInputBindingDescription m_vertexBindingDescription;
	AlignedVector< VkVertexInputAttributeDescription > m_vertexAttributeDescriptions;
	uint32_t m_hash;
};

	}
}

