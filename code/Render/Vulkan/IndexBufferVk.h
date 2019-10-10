#pragma once

#include "Render/IndexBuffer.h"
#include "Render/Vulkan/ApiHeader.h"

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

