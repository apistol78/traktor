/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>

#include "Physics/HingeJoint.h"
#include "Physics/Jolt/JointJolt.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_JOLT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

/*!
 * \ingroup Jolt
 */
class T_DLLCLASS HingeJointJolt : public JointJolt< HingeJoint, JPH::HingeConstraint >
{
	T_RTTI_CLASS;

public:
	explicit HingeJointJolt(IWorldCallback* callback, JPH::HingeConstraint* constraint, BodyJolt* body1, BodyJolt* body2);

	virtual Vector4 getAnchor() const override final;

	virtual Vector4 getAxis() const override final;

	virtual float getAngle() const override final;

	virtual float getAngleVelocity() const override final;

	virtual void setMotor(float targetVelocity, float maxImpulse) override final;
};

}
