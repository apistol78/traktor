/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Physics/BallJoint.h"
#include "Physics/Bullet/JointBullet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_BULLET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS BallJointBullet : public JointBullet< BallJoint, btPoint2PointConstraint >
{
	T_RTTI_CLASS;

public:
	BallJointBullet(IWorldCallback* callback, btPoint2PointConstraint* constraint, BodyBullet* body1, BodyBullet* body2);

	virtual void setAnchor(const Vector4& anchor) override final;

	virtual Vector4 getAnchor() const override final;
};

	}
}

