/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
	const Transform jointTransform[2],
	const Transform body1Transform[2],
	const Transform body2Transform[2],
	const JointDesc* jointDesc
) const
{
	const HingeJointDesc* hingeJointDesc = mandatory_non_null_type_cast< const HingeJointDesc* >(jointDesc);
	const Scalar c_axisLength(5.0f);

	primitiveRenderer->pushWorld(jointTransform[1].toMatrix44());

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
