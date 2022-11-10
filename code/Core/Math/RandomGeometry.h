/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cmath>
#include "Core/Math/Random.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Const.h"

namespace traktor
{

/*! Random geometry.
 * \ingroup Core
 */
class RandomGeometry : public Random
{
public:
	RandomGeometry(uint32_t seed = 5489UL)
	:	Random(seed)
	{
	}

	inline Vector4 nextUnit()
	{
		double z = 2.0 * nextDouble() - 1.0;
		double t = 2.0 * PI * nextDouble();
		double w = std::sqrt(1.0 - z * z);
		double x = w * std::cos(t);
		double y = w * std::sin(t);
		return Vector4(
			float(x),
			float(y),
			float(z),
			0.0f
		);
	}

	inline Vector4 nextHemi(const Vector4& direction)
	{
		Vector4 out = nextUnit();
		if (dot3(out, direction.xyz0()) < 0.0f)
			out = -out;
		return out;
	}
};

}

