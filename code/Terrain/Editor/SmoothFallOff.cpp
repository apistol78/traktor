/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Terrain/Editor/SmoothFallOff.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.SmoothFallOff", SmoothFallOff, IFallOff)

double SmoothFallOff::evaluate(double x, double y) const
{
	const double d = x * x + y * y;
	if (d >= 1.0)
		return 0.0;

	const double v = 1.0f - sqrtf(d);
	return clamp(sin(v * PI / 4.0), 0.0, 1.0);
}

}
