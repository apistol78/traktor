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
#include <Jolt/Physics/Constraints/PointConstraint.h>

#include "Physics/BallJoint.h"
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
class T_DLLCLASS BallJointJolt : public JointJolt< BallJoint, JPH::PointConstraint >
{
	T_RTTI_CLASS;

public:
	explicit BallJointJolt(IWorldCallback* callback, JPH::PointConstraint* constraint, BodyJolt* body1, BodyJolt* body2);

	virtual void setAnchor(const Vector4& anchor) override final;

	virtual Vector4 getAnchor() const override final;
};

}
