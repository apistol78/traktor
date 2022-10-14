#pragma once

#include "Render/Buffer.h"

namespace traktor::render
{

class Context;

class BufferVk : public Buffer
{
	T_RTTI_CLASS;

public:
	explicit BufferVk(Context* context, uint32_t elementCount, uint32_t elementSize, uint32_t& instances);

	virtual ~BufferVk();

protected:
	Context* m_context = nullptr;
	uint32_t& m_instances;
};

}
