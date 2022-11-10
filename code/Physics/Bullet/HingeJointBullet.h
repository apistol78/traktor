/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Physics/HingeJoint.h"
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
class T_DLLCLASS HingeJointBullet : public JointBullet< HingeJoint, btHingeConstraint >
{
	T_RTTI_CLASS;

public:
	HingeJointBullet(IWorldCallback* callback, btHingeConstraint* constraint, BodyBullet* body1, BodyBullet* body2);

	virtual Vector4 getAnchor() const override final;

	virtual Vector4 getAxis() const override final;

	virtual float getAngle() const override final;

	virtual float getAngleVelocity() const override final;

	virtual void setMotor(float targetVelocity, float maxImpulse) override final;
};

	}
}

