#pragma once

#include "Spray/Editor/SourceRenderer.h"

namespace traktor
{
	namespace spray
	{

class QuadSourceRenderer : public SourceRenderer
{
	T_RTTI_CLASS;

public:
	virtual void render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const override final;
};

	}
}

