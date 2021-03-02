#pragma once

#include "Physics/Editor/IPhysicsJointRenderer.h"

namespace traktor
{
	namespace physics
	{

class ConeTwistJointRenderer : public IPhysicsJointRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getDescType() const override final;

	virtual void draw(
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform jointTransform[2],
		const Transform body1Transform[2],
		const Transform body2Transform[2],
		const JointDesc* jointDesc
	) const override final;
};

	}
}

