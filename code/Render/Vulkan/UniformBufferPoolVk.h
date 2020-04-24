#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{
	
class UniformBufferPoolVk : public Object
{
	T_RTTI_CLASS;

public:
	enum { MaxPendingFrames = 8 };

	UniformBufferPoolVk(VkDevice logicalDevice, VmaAllocator allocator);

	bool acquire(
		uint32_t size,
		VkBuffer& inoutBuffer,
		VmaAllocation& inoutAllocation,
		void*& inoutMappedPtr
	);

	void collect();

private:
	struct BufferChain
	{
		uint32_t size;
		VkBuffer buffer;
		VmaAllocation allocation;
		void* mappedPtr;
	};

	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	AlignedVector< BufferChain > m_free[64];
	AlignedVector< BufferChain > m_released[MaxPendingFrames];
	uint32_t m_counter;
};

	}
}
