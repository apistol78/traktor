/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Physics/Hinge2Joint.h"
#include "Physics/Bullet/JointBullet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_BULLET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS Hinge2JointBullet : public JointBullet< Hinge2Joint, btHinge2Constraint >
{
	T_RTTI_CLASS;

public:
	explicit Hinge2JointBullet(IWorldCallback* callback, btHinge2Constraint* constraint, BodyBullet* body1, BodyBullet* body2);

	virtual void addTorques(float torqueAxis1, float torqueAxis2) override final;

	virtual float getAngleAxis1() const override final;

	virtual void setVelocityAxis1(float velocityAxis1) override final;

	virtual void setVelocityAxis2(float velocityAxis2) override final;

	virtual void getAnchors(Vector4& outAnchor1, Vector4& outAnchor2) const override final;
};

}
