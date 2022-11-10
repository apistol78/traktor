/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

namespace traktor
{

/*! \ingroup Core */
//@{

inline bool isNan(float value)
{
	uint32_t bits = *reinterpret_cast< const uint32_t* >(&value);
	return ((bits & 0x7f800000) >> 23) == 0xff && (bits & 0x7fffff) != 0;
}

inline bool isInfinite(float value)
{
	uint32_t bits = *reinterpret_cast< const uint32_t* >(&value);
	return (bits & 0x7f800000) == 0x7f800000 && (bits & 0x7fffff) == 0;
}

inline bool isNanOrInfinite(float value)
{
	return isNan(value) || isInfinite(value);
}

inline float lerp(float a, float b, float c)
{
	return a * (1.0f - c) + b * c;
}

inline double lerp(double a, double b, double c)
{
	return a * (1.0 - c) + b * c;
}

//@}

}

