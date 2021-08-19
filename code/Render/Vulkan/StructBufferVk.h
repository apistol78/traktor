#pragma once

#include "Render/StructBuffer.h"

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

protected:
	Context* m_context = nullptr;
	uint32_t& m_instances;
};

	}
}
