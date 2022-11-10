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

/*! 2 axis hinge joint.
 * \ingroup Physics
 */
class T_DLLCLASS Hinge2Joint : public Joint
{
	T_RTTI_CLASS;

public:
	virtual void addTorques(float torqueAxis1, float torqueAxis2) = 0;

	virtual float getAngleAxis1() const = 0;

	virtual void setVelocityAxis1(float velocityAxis1) = 0;

	virtual void setVelocityAxis2(float velocityAxis2) = 0;

	virtual void getAnchors(Vector4& outAnchor1, Vector4& outAnchor2) const = 0;
};

	}
}

