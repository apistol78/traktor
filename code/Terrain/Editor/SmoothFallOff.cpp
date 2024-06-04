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

SmoothFallOff::SmoothFallOff()
{
	for (int32_t i = 0; i < 100; ++i)
	{
		const float f = i / (100.0f - 1.0f);
		const float v = clamp(1.0f - (float)sin(f * f * PI / 4.0), 0.0f, 1.0f);
		m_lut[i] = v;
	}
}

float SmoothFallOff::evaluate(float x, float y) const
{
	const float d = x * x + y * y;
	if (d < 1.0f)
	{
		const int32_t index = int32_t(d * (100 - 1));
		return m_lut[index];
	}
	else
		return 0.0f;
}

}
