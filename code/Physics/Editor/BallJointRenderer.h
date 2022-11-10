/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Physics/Editor/IPhysicsJointRenderer.h"

namespace traktor
{
	namespace physics
	{

class BallJointRenderer : public IPhysicsJointRenderer
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

