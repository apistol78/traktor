#pragma once

#include "Core/Ref.h"
#include "Render/Vulkan/BufferViewVk.h"
#include "Render/Vulkan/StructBufferVk.h"
#include "Render/Vulkan/Private/Buffer.h"

namespace traktor
{
	namespace render
	{

class StructBufferDynamicVk : public StructBufferVk
{
	T_RTTI_CLASS;

public:
	explicit StructBufferDynamicVk(Context* context, uint32_t bufferSize, uint32_t& instances);

	virtual ~StructBufferDynamicVk();

	bool create(int32_t inFlightCount);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	virtual const IBufferView* getBufferView() const override final;

private:
	Ref< Buffer > m_buffer;
	BufferViewVk* m_bufferViews = nullptr;
	uint32_t m_inFlightCount = 0;
	uint32_t m_range = 0;
	uint32_t m_index = 0;
	uint32_t m_view = 0;
	uint8_t* m_ptr = nullptr;
};

	}
}
