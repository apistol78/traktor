/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

namespace traktor::terrain
{

/*! Brush falloff.
 * \ingroup Terrain
 */
class IFallOff : public Object
{
	T_RTTI_CLASS;

public:
	/*! Evaluate brush falloff.
	 *
	 * \param x Brush X coordinate, -1 to 1.
	 * \param y Brush Y coordinate, -1 to 1.
	 * \return Falloff value, 0 no brush influence, 1 full brush influence.
	 */
	virtual float evaluate(float x, float y) const = 0;
};

}
