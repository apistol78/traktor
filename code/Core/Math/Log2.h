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

/*! Get log-2 value.
 * \ingroup Core
 */
int32_t T_FORCE_INLINE log2(int32_t v)
{
	for (int i = 31; i >= 0; --i)
	{
		if (v & (1 << i))
			return i;
	}
	return 0;
}

/*! Check if value is a log-2.
 * \ingroup Core
 */
bool T_FORCE_INLINE isLog2(int32_t v)
{
	return (1 << log2(v)) == v;
}

/*! Get nearest log-2.
 * \ingroup Core
 */
uint32_t T_FORCE_INLINE nearestLog2(uint32_t num)
{
	uint32_t n = num > 0 ? num - 1 : 0;

	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;

	return n;
}

/*! Get previous log-2.
 * \ingroup Core
 */
uint32_t T_FORCE_INLINE previousLog2(uint32_t num)
{
	uint32_t lg2 = nearestLog2(num);
	return lg2 > 1 ? (lg2 >> 1) : 1;
}

}
