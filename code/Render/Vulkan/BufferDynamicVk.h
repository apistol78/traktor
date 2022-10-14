#pragma once

#include "Core/Ref.h"
#include "Render/Vulkan/BufferViewVk.h"
#include "Render/Vulkan/BufferVk.h"
#include "Render/Vulkan/Private/ApiBuffer.h"

namespace traktor::render
{

class BufferDynamicVk : public BufferVk
{
	T_RTTI_CLASS;

public:
	explicit BufferDynamicVk(Context* context, uint32_t elementCount, uint32_t elementSize, uint32_t& instances);

	virtual ~BufferDynamicVk();

	bool create(uint32_t usageBits, int32_t inFlightCount);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	virtual const IBufferView* getBufferView() const override final;

private:
	Ref< ApiBuffer > m_buffer;
	BufferViewVk* m_bufferViews = nullptr;
	uint32_t m_inFlightCount = 0;
	uint32_t m_range = 0;
	uint32_t m_index = 0;
	uint32_t m_view = 0;
	uint8_t* m_ptr = nullptr;
};

}
