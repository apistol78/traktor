/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/SharpFallOff.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.SharpFallOff", SharpFallOff, IFallOff)

float SharpFallOff::evaluate(float x, float y) const
{
	const float d = x * x + y * y;
	if (d >= 1.0f)
		return 0.0f;
	else
		return 1.0f;
}

}
