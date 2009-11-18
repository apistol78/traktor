#ifndef traktor_spray_SphereSourceRenderer_H
#define traktor_spray_SphereSourceRenderer_H

#include "Spray/Editor/SourceRenderer.h"

namespace traktor
{
	namespace spray
	{

class SphereSourceRenderer : public SourceRenderer
{
	T_RTTI_CLASS;

public:
	virtual void render(render::PrimitiveRenderer* primitiveRenderer, Source* source) const;
};

	}
}

#endif	// traktor_spray_SphereSourceRenderer_H
