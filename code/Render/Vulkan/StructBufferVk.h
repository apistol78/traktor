#pragma once

#include "Render/StructBuffer.h"
#include "Render/Vulkan/Buffer.h"

namespace traktor
{
	namespace render
	{

class StructBufferVk : public StructBuffer
{
	T_RTTI_CLASS;

public:
	StructBufferVk(uint32_t bufferSize);

	bool create(VmaAllocator allocator, int32_t inFlightCount);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t structOffset, uint32_t structCount) override final;

	virtual void unlock() override final;

	VkBuffer getVkBuffer() const { return m_buffers[m_index]; }

private:
	AlignedVector< Buffer > m_buffers;
	int32_t m_index;
};

	}
}
