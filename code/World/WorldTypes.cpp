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
	const Vector2 kernelSize(0.5f, 0.5f);
	return Vector2(
		(float)((count / 2) & 1) * kernelSize.x - (2.0f * kernelSize.x) / 3.0f,
		(float)(count & 1) * kernelSize.y - (2.0f * kernelSize.y) / 3.0f
	);
}

}
