#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class Context;
	
class UniformBufferPool : public Object
{
	T_RTTI_CLASS;

public:
	enum { MaxPendingFrames = 8 };

	explicit UniformBufferPool(Context* context);

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

	Context* m_context = nullptr;
	AlignedVector< BufferChain > m_free[64];
	AlignedVector< BufferChain > m_released[MaxPendingFrames];
	uint32_t m_counter;
};

	}
}
