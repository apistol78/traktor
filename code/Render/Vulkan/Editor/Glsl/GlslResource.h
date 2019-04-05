#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

class GlslResource : public Object
{
	T_RTTI_CLASS;

public:
	GlslResource();

	int32_t getBinding() const { return m_binding; }

private:
	friend class GlslLayout;

	int32_t m_binding;
};

	}
}
