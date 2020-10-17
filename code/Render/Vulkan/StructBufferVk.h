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
	StructBufferVk(
		uint32_t bufferSize,
		Buffer&& buffer
	);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t structOffset, uint32_t structCount) override final;

	virtual void unlock() override final;

	VkBuffer getVkBuffer() const { return m_buffer; }

private:
	Buffer m_buffer;
};

	}
}
