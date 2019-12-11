#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

class PrimitiveRenderer;

	}

	namespace spray
	{

class SourceData;

/*! Emitter source renderer. */
class SourceRenderer : public Object
{
	T_RTTI_CLASS;

public:
	virtual void render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const = 0;
};

	}
}

