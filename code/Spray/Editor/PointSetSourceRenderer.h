#ifndef traktor_spray_PointSetSourceRenderer_H
#define traktor_spray_PointSetSourceRenderer_H

#include "Spray/Editor/SourceRenderer.h"

namespace traktor
{
	namespace spray
	{

class PointSetSourceRenderer : public SourceRenderer
{
	T_RTTI_CLASS;

public:
	virtual void render(render::PrimitiveRenderer* primitiveRenderer, const Source* source) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_spray_PointSetSourceRenderer_H
