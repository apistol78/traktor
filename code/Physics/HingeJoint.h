/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector4.h"
#include "Physics/Joint.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! Hinge joint.
 * \ingroup Physics
 */
class T_DLLCLASS HingeJoint : public Joint
{
	T_RTTI_CLASS;

public:
	virtual Vector4 getAnchor() const = 0;

	virtual Vector4 getAxis() const = 0;

	virtual float getAngle() const = 0;

	virtual float getAngleVelocity() const = 0;

	virtual void setMotor(float targetVelocity, float maxImpulse) = 0;
};

	}
}

