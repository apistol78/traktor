#pragma once

#include "Core/Ref.h"
#include "Render/IndexBuffer.h"
#include "Render/Vulkan/Private/Buffer.h"

namespace traktor
{
	namespace render
	{

class Context;

/*!
 * \ingroup Vulkan
 */
class IndexBufferVk : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferVk(
		Context* context,
		IndexType indexType,
		uint32_t bufferSize
	);

	bool create();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	VkBuffer getVkBuffer() const { return *m_buffer; }

private:
	Context* m_context;
	Ref< Buffer > m_buffer;
};

	}
}

