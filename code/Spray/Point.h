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
#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace spray
	{

/*! Particle point.
 * \ingroup Spray
 */
struct T_MATH_ALIGN16 Point
{
	Vector4 position;
	Vector4 velocity;
	Vector4 color;		// 48
	float orientation;
	float angularVelocity;
	float inverseMass;
	float age;
	float maxAge;
	float size;
	float random;		// 28
};

/*! Array of particles.
 * \ingroup Spray
 */
typedef AlignedVector< Point > pointVector_t;

	}
}

