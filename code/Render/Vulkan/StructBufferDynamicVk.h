#pragma once

#include "Core/Ref.h"
#include "Render/Vulkan/StructBufferVk.h"
#include "Render/Vulkan/Private/Buffer.h"

namespace traktor
{
	namespace render
	{

class Buffer;

class StructBufferDynamicVk : public StructBufferVk
{
	T_RTTI_CLASS;

public:
	explicit StructBufferDynamicVk(Context* context, uint32_t bufferSize, uint32_t& instances);

	virtual ~StructBufferDynamicVk();

	bool create(int32_t inFlightCount);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t structOffset, uint32_t structCount) override final;

	virtual void unlock() override final;

	virtual VkBuffer getVkBuffer() const override final { return *m_buffer; }

	virtual uint32_t getVkBufferOffset() const override final { return m_offset; }

private:
	Ref< Buffer > m_buffer;
	uint32_t m_alignedBufferSize = 0;
	int32_t m_inFlightCount = 0;
	int32_t m_index = 0;
	uint32_t m_offset = 0;
};

	}
}
