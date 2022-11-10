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

namespace traktor
{
	namespace weather
	{

struct CloudParticle
{
	Vector4 position;
	float positionVelocity;
	float maxRadius;
	float radius;
	float rotation;
	float rotationVelocity;
	float opacity;
	int sprite;
};

	}
}

