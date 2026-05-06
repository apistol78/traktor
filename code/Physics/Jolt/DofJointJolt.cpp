/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Jolt/DofJointJolt.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.DofJointJolt", DofJointJolt, DofJoint)

DofJointJolt::DofJointJolt(IWorldCallback* callback, JPH::SixDOFConstraint* constraint, BodyJolt* body1, BodyJolt* body2)
:	JointJolt< DofJoint, JPH::SixDOFConstraint >(callback, constraint, body1, body2)
{
}

}
