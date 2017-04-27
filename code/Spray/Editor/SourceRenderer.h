/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

class SourceData;

/*! \brief Emitter source renderer. */
class SourceRenderer : public Object
{
	T_RTTI_CLASS;

public:
	virtual void render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const = 0;
};

	}
}

#endif	// traktor_spray_SourceRenderer_H
