#ifndef traktor_spray_SourceRenderer_H
#define traktor_spray_SourceRenderer_H

#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

class PrimitiveRenderer;

	}

	namespace spray
	{

class Source;

/*! \brief Emitter source renderer. */
class SourceRenderer : public Object
{
	T_RTTI_CLASS(EmitterRenderer)

public:
	virtual void render(render::PrimitiveRenderer* primitiveRenderer, Source* source) const = 0;
};

	}
}

#endif	// traktor_spray_SourceRenderer_H
