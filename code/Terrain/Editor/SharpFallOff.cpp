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

double SharpFallOff::evaluate(double x, double y) const
{
	const double d = x * x + y * y;
	if (d >= 1.0)
		return 0.0;
	else
		return 1.0;
}

}
