/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_BoxShapeRenderer_H
#define traktor_physics_BoxShapeRenderer_H

#include "Physics/Editor/IPhysicsShapeRenderer.h"

namespace traktor
{
	namespace physics
	{

class BoxShapeRenderer : public IPhysicsShapeRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getDescType() const override final;

	virtual void draw(
		resource::IResourceManager* resourceManager,
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform& body1Transform0,
		const Transform& body1Transform,
		const ShapeDesc* shapeDesc
	) const override final;
};

	}
}

#endif	// traktor_physics_BoxShapeRenderer_H
