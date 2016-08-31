#ifndef traktor_spray_BoxSourceRenderer_H
#define traktor_spray_BoxSourceRenderer_H

#include "Spray/Editor/SourceRenderer.h"

namespace traktor
{
	namespace spray
	{

class BoxSourceRenderer : public SourceRenderer
{
	T_RTTI_CLASS;

public:
	virtual void render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_spray_BoxSourceRenderer_H
