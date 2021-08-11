#pragma once

#include "Render/StructBuffer.h"
#include "Render/Vulkan/Private/ApiHeader.h"
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

	VkBuffer getVkBuffer() const { return *m_buffer; }

	uint32_t getVkBufferOffset() const { return m_offset; }

	uint32_t getVkBufferRange() const { return m_range; }

	uint32_t getVkBufferSize() const { return m_size; }

protected:
	Context* m_context = nullptr;
	uint32_t& m_instances;
	Ref< Buffer > m_buffer;
	uint32_t m_offset = 0;
	uint32_t m_range = 0;
	uint32_t m_size = 0;
};

	}
}
