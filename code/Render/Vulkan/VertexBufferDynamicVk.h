#pragma once

#include "Core/RefArray.h"
#include "Render/Vulkan/Buffer.h"
#include "Render/Vulkan/VertexBufferVk.h"

namespace traktor
{
	namespace render
	{

class Context;

/*!
 * \ingroup Vulkan
 */
class VertexBufferDynamicVk : public VertexBufferVk
{
	T_RTTI_CLASS;

public:
	VertexBufferDynamicVk(
		Context* context,
		uint32_t bufferSize,
		const VkVertexInputBindingDescription& vertexBindingDescription,
		const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
		uint32_t hash
	);

	bool create(int32_t inFlightCount);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) override final;

	virtual void unlock() override final;

	virtual VkBuffer getVkBuffer() const override final;

private:
	Ref< Context > m_context;
	RefArray< Buffer > m_buffers;
	int32_t m_index;
};

	}
}

