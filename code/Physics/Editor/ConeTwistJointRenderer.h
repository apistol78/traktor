#ifndef traktor_physics_ConeTwistJointRenderer_H
#define traktor_physics_ConeTwistJointRenderer_H

#include "Physics/Editor/IPhysicsJointRenderer.h"

namespace traktor
{
	namespace physics
	{

class ConeTwistJointRenderer : public IPhysicsJointRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getDescType() const T_OVERRIDE T_FINAL;

	virtual void draw(
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform& body1Transform0,
		const Transform& body1Transform,
		const JointDesc* jointDesc
	) const T_OVERRIDE T_FINAL;

	virtual void draw(
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform& body1Transform0,
		const Transform& body1Transform,
		const Transform& body2Transform0,
		const Transform& body2Transform,
		const JointDesc* jointDesc
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_physics_ConeTwistJointRenderer_H
