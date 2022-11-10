/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Bullet/AxisJointBullet.h"
#include "Physics/Bullet/Conversion.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.AxisJointBullet", AxisJointBullet, AxisJoint)

AxisJointBullet::AxisJointBullet(IWorldCallback* callback, btHingeConstraint* constraint, BodyBullet* body1, BodyBullet* body2)
:	JointBullet< AxisJoint, btHingeConstraint >(callback, constraint, body1, body2)
{
}

Vector4 AxisJointBullet::getAxis() const
{
	return fromBtVector3(m_constraint->getAFrame().getBasis().getColumn(2), 0.0f);
}

	}
}
