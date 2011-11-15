#ifndef traktor_physics_HeightfieldShapeRenderer_H
#define traktor_physics_HeightfieldShapeRenderer_H

#include "Physics/Editor/IPhysicsShapeRenderer.h"

namespace traktor
{
	namespace physics
	{

class HeightfieldShapeRenderer : public IPhysicsShapeRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getDescType() const;

	virtual void draw(
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform& body1Transform0,
		const Transform& body1Transform,
		const ShapeDesc* shapeDesc
	) const;
};

	}
}

#endif	// traktor_physics_HeightfieldShapeRenderer_H
