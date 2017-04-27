/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/HingeJointDesc.h"
#include "Physics/Editor/HingeJointRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.HingeJointRenderer", 0, HingeJointRenderer, IPhysicsJointRenderer)

const TypeInfo& HingeJointRenderer::getDescType() const
{
	return type_of< HingeJointDesc >();
}

void HingeJointRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const JointDesc* jointDesc
) const
{
	const HingeJointDesc* hingeJointDesc = checked_type_cast< const HingeJointDesc*, false >(jointDesc);

	const Scalar c_axisLength(5.0f);

	Vector4 jointAnchor = hingeJointDesc->getAnchor().xyz1();
	Vector4 jointAxis = hingeJointDesc->getAxis().xyz0();

	primitiveRenderer->drawLine(
		jointAnchor - jointAxis * c_axisLength,
		jointAnchor + jointAxis * c_axisLength,
		Color4ub(255, 255, 0)
	);
}

void HingeJointRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const Transform& body2Transform0,
	const Transform& body2Transform,
	const JointDesc* jointDesc
) const
{
	const HingeJointDesc* hingeJointDesc = checked_type_cast< const HingeJointDesc*, false >(jointDesc);

	const Scalar c_axisLength(5.0f);

	primitiveRenderer->pushWorld((body1Transform * body1Transform0.inverse()).toMatrix44());
	
	Vector4 jointAnchor = hingeJointDesc->getAnchor().xyz1();
	Vector4 jointAxis = hingeJointDesc->getAxis().xyz0();

	primitiveRenderer->drawLine(
		jointAnchor - jointAxis * c_axisLength,
		jointAnchor + jointAxis * c_axisLength,
		Color4ub(255, 255, 0)
	);

	primitiveRenderer->popWorld();
}

	}
}
