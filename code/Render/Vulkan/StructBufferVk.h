#pragma once

#include "Render/StructBuffer.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class StructBufferVk : public StructBuffer
{
	T_RTTI_CLASS;

public:
	StructBufferVk(
		uint32_t bufferSize,
		VmaAllocator allocator,
		VmaAllocation allocation,
		VkBuffer storageBuffer
	);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t structOffset, uint32_t structCount) override final;

	virtual void unlock() override final;

	VkBuffer getVkBuffer() const { return m_storageBuffer; }

private:
	VmaAllocator m_allocator;
	VmaAllocation m_allocation;
	VkBuffer m_storageBuffer;
	bool m_locked;
};

	}
}
