/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/WorldTypes.h"

namespace traktor::world
{

const EntityState EntityState::None		= { false, false, false };
const EntityState EntityState::Visible	= {  true, false, false };
const EntityState EntityState::Dynamic	= { false,  true, false };
const EntityState EntityState::Locked	= { false, false,  true };
const EntityState EntityState::All		= {  true,  true,  true };

Vector2 jitter(int32_t count)
{
	const static Vector2 c_halton[] =
	{
		Vector2(0.500000f, 0.333333f),
		Vector2(0.250000f, 0.666667f),
		Vector2(0.750000f, 0.111111f),
		Vector2(0.125000f, 0.444444f),
		Vector2(0.625000f, 0.777778f),
		Vector2(0.375000f, 0.222222f),
		Vector2(0.875000f, 0.555556f),
		Vector2(0.062500f, 0.888889f),
		Vector2(0.562500f, 0.037037f),
		Vector2(0.312500f, 0.370370f),
		Vector2(0.812500f, 0.703704f),
		Vector2(0.187500f, 0.148148f),
		Vector2(0.687500f, 0.481481f),
		Vector2(0.437500f, 0.814815f),
		Vector2(0.937500f, 0.259259f),
		Vector2(0.031250f, 0.592593f)
	};
	return c_halton[count % sizeof_array(c_halton)] - Vector2(0.5f, 0.5f);
}

}
