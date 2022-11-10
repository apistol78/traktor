/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
	const Transform jointTransform[2],
	const Transform body1Transform[2],
	const Transform body2Transform[2],
	const JointDesc* jointDesc
) const
{
	const ConeTwistJointDesc* coneTwistJointDesc = mandatory_non_null_type_cast< const ConeTwistJointDesc* >(jointDesc);

	//const Scalar c_axisLength(5.0f);
	//const float c_limitLength = 2.0f;

	//Vector4 jointAnchor = coneTwistJointDesc->getAnchor().xyz1();

	//primitiveRenderer->drawSolidPoint(jointAnchor, 4.0f, Color4ub(255, 255, 255));

	//primitiveRenderer->drawLine(
	//	jointAnchor - coneTwistJointDesc->getTwistAxis() * c_axisLength,
	//	jointAnchor + coneTwistJointDesc->getTwistAxis() * c_axisLength,
	//	Color4ub(255, 255, 0)
	//);
	//primitiveRenderer->drawLine(
	//	jointAnchor - coneTwistJointDesc->getConeAxis() * c_axisLength,
	//	jointAnchor + coneTwistJointDesc->getConeAxis() * c_axisLength,
	//	Color4ub(0, 255, 255)
	//);
	////primitiveRenderer->drawLine(
	////	body1Center,
	////	jointAnchor,
	////	Color4ub(255, 255, 255)
	////);
	////primitiveRenderer->drawLine(
	////	body2Center,
	////	jointAnchor,
	////	Color4ub(255, 255, 255)
	////);

	//float coneAngle1, coneAngle2;
	//coneTwistJointDesc->getConeAngles(coneAngle1, coneAngle2);

	//Vector4 bac1 = coneTwistJointDesc->getTwistAxis().normalized();
	//Vector4 coneAxis1 = coneTwistJointDesc->getConeAxis().normalized();
	//Vector4 coneAxis2 = cross(bac1, coneAxis1).normalized();

	//primitiveRenderer->drawCone(
	//	Matrix44(
	//		coneAxis1,
	//		coneAxis2,
	//		bac1,
	//		jointAnchor
	//	),
	//	coneAngle1,
	//	coneAngle2,
	//	c_limitLength,
	//	Color4ub(255, 255, 0, 220),
	//	Color4ub(255, 255, 0, 128)
	//);

	//float twistAngle = coneTwistJointDesc->getTwistAngle() / 2.0f;

	//primitiveRenderer->drawProtractor(
	//	jointAnchor,
	//	coneAxis1,
	//	coneAxis2,
	//	-twistAngle,
	//	twistAngle,
	//	deg2rad(4.0f),
	//	2.0f,
	//	Color4ub(255, 255, 0, 220),
	//	Color4ub(255, 255, 0, 128)
	//);
}

	}
}
