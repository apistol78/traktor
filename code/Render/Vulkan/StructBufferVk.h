#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Render/StructBuffer.h"
#include "Render/Vulkan/Private/Buffer.h"

namespace traktor
{
	namespace render
	{

class Context;

class StructBufferVk : public StructBuffer
{
	T_RTTI_CLASS;

public:
	explicit StructBufferVk(Context* context, uint32_t bufferSize, uint32_t& instances);

	virtual ~StructBufferVk();

	bool create(int32_t inFlightCount);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t structOffset, uint32_t structCount) override final;

	virtual void unlock() override final;

	VkBuffer getVkBuffer() const { return *m_buffer; }

	uint32_t getVkBufferOffset() const { return m_offset; }

private:
	Context* m_context = nullptr;
	Ref< Buffer > m_buffer;
	uint32_t& m_instances;
	uint32_t m_alignedBufferSize = 0;
	int32_t m_inFlightCount = 0;
	int32_t m_index = 0;
	uint32_t m_offset = 0;
};

	}
}
