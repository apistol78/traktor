/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

float SmoothFallOff::evaluate(float x, float y) const
{
	const float d = x * x + y * y;
	if (d >= 1.0f)
		return 0.0f;

	const float v = 1.0f - sqrtf(d);
	return clamp(sinf(v * PI / 4.0f), 0.0f, 1.0f);
}

}
