/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/BallJointDesc.h"
#include "Physics/Editor/BallJointRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.BallJointRenderer", 0, BallJointRenderer, IPhysicsJointRenderer)

const TypeInfo& BallJointRenderer::getDescType() const
{
	return type_of< BallJointDesc >();
}

void BallJointRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform jointTransform[2],
	const Transform body1Transform[2],
	const Transform body2Transform[2],
	const JointDesc* jointDesc
) const
{
	const BallJointDesc* ballJointDesc = mandatory_non_null_type_cast< const BallJointDesc* >(jointDesc);
	Vector4 jointAnchorW = jointTransform[1] * ballJointDesc->getAnchor().xyz1();
	primitiveRenderer->drawSolidPoint(jointAnchorW, 4.0f, Color4ub(255, 255, 255));
}

	}
}
