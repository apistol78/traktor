#ifndef traktor_spray_SplineSourceRenderer_H
#define traktor_spray_SplineSourceRenderer_H

#include "Spray/Editor/SourceRenderer.h"

namespace traktor
{
	namespace spray
	{

class SplineSourceRenderer : public SourceRenderer
{
	T_RTTI_CLASS;

public:
	virtual void render(render::PrimitiveRenderer* primitiveRenderer, Source* source) const;
};

	}
}

#endif	// traktor_spray_SplineSourceRenderer_H
