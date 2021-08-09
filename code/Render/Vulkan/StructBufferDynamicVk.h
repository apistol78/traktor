#pragma once

#include "Core/Ref.h"
#include "Render/Vulkan/StructBufferVk.h"

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

private:
	uint32_t m_alignedBufferSize = 0;
	int32_t m_inFlightCount = 0;
	int32_t m_index = 0;
	uint8_t* m_ptr = nullptr;
};

	}
}
