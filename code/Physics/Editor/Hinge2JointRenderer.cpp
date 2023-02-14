/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Hinge2JointDesc.h"
#include "Physics/Editor/Hinge2JointRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.Hinge2JointRenderer", 0, Hinge2JointRenderer, IPhysicsJointRenderer)

const TypeInfo& Hinge2JointRenderer::getDescType() const
{
	return type_of< Hinge2JointDesc >();
}

void Hinge2JointRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform jointTransform[2],
	const Transform body1Transform[2],
	const Transform body2Transform[2],
	const JointDesc* jointDesc
) const
{
	const Hinge2JointDesc* hinge2JointDesc = mandatory_non_null_type_cast< const Hinge2JointDesc* >(jointDesc);

	const Scalar c_axis1Length(5.0f);
	const Scalar c_axis2Length(1.0f);

	//Vector4 jointAnchor = hinge2JointDesc->getAnchor().xyz1();
	//Vector4 jointAxis1 = hinge2JointDesc->getAxis1().xyz0();
	//Vector4 jointAxis2 = hinge2JointDesc->getAxis2().xyz0();

	//primitiveRenderer->drawLine(
	//	jointAnchor - jointAxis1 * c_axis1Length,
	//	jointAnchor + jointAxis1 * c_axis1Length,
	//	Color4ub(255, 255, 0)
	//);
	//primitiveRenderer->drawLine(
	//	jointAnchor - jointAxis2 * c_axis2Length,
	//	jointAnchor + jointAxis2 * c_axis2Length,
	//	Color4ub(255, 255, 0)
	//);
}

}
