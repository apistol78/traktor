/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/Editor/ConeTwistJointRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.ConeTwistJointRenderer", 0, ConeTwistJointRenderer, IPhysicsJointRenderer)

const TypeInfo& ConeTwistJointRenderer::getDescType() const
{
	return type_of< ConeTwistJointDesc >();
}

void ConeTwistJointRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const JointDesc* jointDesc
) const
{
	const ConeTwistJointDesc* coneTwistJointDesc = checked_type_cast< const ConeTwistJointDesc*, false >(jointDesc);

	const Scalar c_axisLength(5.0f);
	const float c_limitLength = 2.0f;

	Vector4 jointAnchor = coneTwistJointDesc->getAnchor().xyz1();

	primitiveRenderer->drawSolidPoint(jointAnchor, 4.0f, Color4ub(255, 255, 255));

	primitiveRenderer->drawLine(
		jointAnchor - coneTwistJointDesc->getTwistAxis() * c_axisLength,
		jointAnchor + coneTwistJointDesc->getTwistAxis() * c_axisLength,
		Color4ub(255, 255, 0)
	);
	primitiveRenderer->drawLine(
		jointAnchor - coneTwistJointDesc->getConeAxis() * c_axisLength,
		jointAnchor + coneTwistJointDesc->getConeAxis() * c_axisLength,
		Color4ub(0, 255, 255)
	);
	//primitiveRenderer->drawLine(
	//	body1Center,
	//	jointAnchor,
	//	Color4ub(255, 255, 255)
	//);
	//primitiveRenderer->drawLine(
	//	body2Center,
	//	jointAnchor,
	//	Color4ub(255, 255, 255)
	//);

	float coneAngle1, coneAngle2;
	coneTwistJointDesc->getConeAngles(coneAngle1, coneAngle2);

	Vector4 bac1 = coneTwistJointDesc->getTwistAxis().normalized();
	Vector4 coneAxis1 = coneTwistJointDesc->getConeAxis().normalized();
	Vector4 coneAxis2 = cross(bac1, coneAxis1).normalized();

	primitiveRenderer->drawCone(
		Matrix44(
			coneAxis1,
			coneAxis2,
			bac1,
			jointAnchor
		),
		coneAngle1,
		coneAngle2,
		c_limitLength,
		Color4ub(255, 255, 0, 220),
		Color4ub(255, 255, 0, 128)
	);

	float twistAngle = coneTwistJointDesc->getTwistAngle() / 2.0f;

	primitiveRenderer->drawProtractor(
		jointAnchor,
		coneAxis1,
		coneAxis2,
		-twistAngle,
		twistAngle,
		deg2rad(4.0f),
		2.0f,
		Color4ub(255, 255, 0, 220),
		Color4ub(255, 255, 0, 128)
	);
}

void ConeTwistJointRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const Transform& body2Transform0,
	const Transform& body2Transform,
	const JointDesc* jointDesc
) const
{
	const ConeTwistJointDesc* coneTwistJointDesc = checked_type_cast< const ConeTwistJointDesc*, false >(jointDesc);

	const Scalar c_axisLength(5.0f);
	const float c_limitLength = 2.0f;

	primitiveRenderer->pushWorld((body1Transform * body1Transform0.inverse()).toMatrix44());

	Vector4 jointAnchor = coneTwistJointDesc->getAnchor().xyz1();

	primitiveRenderer->drawSolidPoint(jointAnchor, 4.0f, Color4ub(255, 255, 255));

	primitiveRenderer->drawLine(
		jointAnchor - coneTwistJointDesc->getTwistAxis() * c_axisLength,
		jointAnchor + coneTwistJointDesc->getTwistAxis() * c_axisLength,
		Color4ub(255, 255, 0)
	);
	primitiveRenderer->drawLine(
		jointAnchor - coneTwistJointDesc->getConeAxis() * c_axisLength,
		jointAnchor + coneTwistJointDesc->getConeAxis() * c_axisLength,
		Color4ub(0, 255, 255)
	);
	//primitiveRenderer->drawLine(
	//	body1Center,
	//	jointAnchor,
	//	Color4ub(255, 255, 255)
	//);
	//primitiveRenderer->drawLine(
	//	body2Center,
	//	jointAnchor,
	//	Color4ub(255, 255, 255)
	//);

	float coneAngle1, coneAngle2;
	coneTwistJointDesc->getConeAngles(coneAngle1, coneAngle2);

	Vector4 bac1 = coneTwistJointDesc->getTwistAxis().normalized();
	Vector4 coneAxis1 = coneTwistJointDesc->getConeAxis().normalized();
	Vector4 coneAxis2 = cross(bac1, coneAxis1).normalized();

	primitiveRenderer->drawCone(
		Matrix44(
			coneAxis1,
			coneAxis2,
			bac1,
			jointAnchor
		),
		coneAngle1,
		coneAngle2,
		c_limitLength,
		Color4ub(255, 255, 0, 220),
		Color4ub(255, 255, 0, 128)
	);

	float twistAngle = coneTwistJointDesc->getTwistAngle() / 2.0f;

	primitiveRenderer->drawProtractor(
		jointAnchor,
		coneAxis1,
		coneAxis2,
		-twistAngle,
		twistAngle,
		deg2rad(4.0f),
		2.0f,
		Color4ub(255, 255, 0, 220),
		Color4ub(255, 255, 0, 128)
	);

	primitiveRenderer->popWorld();
}

	}
}
