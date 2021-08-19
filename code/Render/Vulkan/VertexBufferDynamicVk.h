#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Render/Vulkan/VertexBufferVk.h"

namespace traktor
{
	namespace render
	{

class Buffer;
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
		uint32_t hash,
		uint32_t& instances
	);

	virtual ~VertexBufferDynamicVk();

	bool create(int32_t inFlightCount);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	virtual VkBuffer getVkBuffer() const override final;

private:
	Context* m_context = nullptr;
	RefArray< Buffer > m_buffers;
	int32_t m_index = 0;
};

	}
}

