/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Jolt/ConeTwistJointJolt.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ConeTwistJointJolt", ConeTwistJointJolt, ConeTwistJoint)

ConeTwistJointJolt::ConeTwistJointJolt(IWorldCallback* callback, JPH::SwingTwistConstraint* constraint, BodyJolt* body1, BodyJolt* body2)
:	JointJolt< ConeTwistJoint, JPH::SwingTwistConstraint >(callback, constraint, body1, body2)
{
}

}
