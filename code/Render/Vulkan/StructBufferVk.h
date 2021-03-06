#pragma once

#include "Render/StructBuffer.h"
#include "Render/Vulkan/Private/ApiHeader.h"

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

	virtual VkBuffer getVkBuffer() const = 0;

	virtual uint32_t getVkBufferOffset() const = 0;

protected:
	Context* m_context = nullptr;
	uint32_t& m_instances;
};

	}
}
