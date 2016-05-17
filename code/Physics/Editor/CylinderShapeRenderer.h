#ifndef traktor_physics_CylinderShapeRenderer_H
#define traktor_physics_CylinderShapeRenderer_H

#include "Physics/Editor/IPhysicsShapeRenderer.h"

namespace traktor
{
	namespace physics
	{

class CylinderShapeRenderer : public IPhysicsShapeRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getDescType() const T_OVERRIDE T_FINAL;

	virtual void draw(
		resource::IResourceManager* resourceManager,
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform& body1Transform0,
		const Transform& body1Transform,
		const ShapeDesc* shapeDesc
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_physics_CylinderShapeRenderer_H
