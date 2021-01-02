#pragma once

#include "Render/IndexBuffer.h"
#include "Render/Vulkan/Private/Buffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class IndexBufferVk : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferVk(
		IndexType indexType,
		uint32_t bufferSize,
		Buffer&& buffer
	);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	VkBuffer getVkBuffer() const { return m_buffer; }

private:
	Buffer m_buffer;
};

	}
}

