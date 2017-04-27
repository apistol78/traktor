/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/Hinge2JointDesc.h"
#include "Physics/Editor/Hinge2JointRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.Hinge2JointRenderer", 0, Hinge2JointRenderer, IPhysicsJointRenderer)

const TypeInfo& Hinge2JointRenderer::getDescType() const
{
	return type_of< Hinge2JointDesc >();
}

void Hinge2JointRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const JointDesc* jointDesc
) const
{
	const Hinge2JointDesc* hinge2JointDesc = checked_type_cast< const Hinge2JointDesc*, false >(jointDesc);

	const Scalar c_axis1Length(5.0f);
	const Scalar c_axis2Length(1.0f);
	
	Vector4 jointAnchor = hinge2JointDesc->getAnchor().xyz1();
	Vector4 jointAxis1 = hinge2JointDesc->getAxis1().xyz0();
	Vector4 jointAxis2 = hinge2JointDesc->getAxis2().xyz0();

	primitiveRenderer->drawLine(
		jointAnchor - jointAxis1 * c_axis1Length,
		jointAnchor + jointAxis1 * c_axis1Length,
		Color4ub(255, 255, 0)
	);
	primitiveRenderer->drawLine(
		jointAnchor - jointAxis2 * c_axis2Length,
		jointAnchor + jointAxis2 * c_axis2Length,
		Color4ub(255, 255, 0)
	);
}

void Hinge2JointRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const Transform& body2Transform0,
	const Transform& body2Transform,
	const JointDesc* jointDesc
) const
{
	const Hinge2JointDesc* hinge2JointDesc = checked_type_cast< const Hinge2JointDesc*, false >(jointDesc);

	const Scalar c_axisLength(5.0f);

	primitiveRenderer->pushWorld((body1Transform * body1Transform0.inverse()).toMatrix44());
	
	Vector4 jointAnchor = hinge2JointDesc->getAnchor().xyz1();
	Vector4 jointAxis1 = hinge2JointDesc->getAxis1().xyz0();
	Vector4 jointAxis2 = hinge2JointDesc->getAxis2().xyz0();

	primitiveRenderer->drawLine(
		jointAnchor - jointAxis1 * c_axisLength,
		jointAnchor + jointAxis1 * c_axisLength,
		4,
		Color4ub(255, 255, 0)
	);
	primitiveRenderer->drawLine(
		jointAnchor - jointAxis2 * c_axisLength,
		jointAnchor + jointAxis2 * c_axisLength,
		Color4ub(128, 255, 0)
	);

	primitiveRenderer->popWorld();
}

	}
}
