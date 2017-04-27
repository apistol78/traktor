/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	virtual void render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_spray_PointSetSourceRenderer_H
