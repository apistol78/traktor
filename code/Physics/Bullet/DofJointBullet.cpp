/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Bullet/DofJointBullet.h"
#include "Physics/Bullet/Conversion.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.DofJointBullet", DofJointBullet, DofJoint)

DofJointBullet::DofJointBullet(IWorldCallback* callback, btGeneric6DofConstraint* constraint, BodyBullet* body1, BodyBullet* body2)
:	JointBullet< DofJoint, btGeneric6DofConstraint >(callback, constraint, body1, body2)
{
}

}
