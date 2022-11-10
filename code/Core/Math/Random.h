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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Random number generator.
 * \ingroup Core
 *
 * A random number generator class based on the
 * famous Mersenne Twister algorithm.
 *
 * Based on the code http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/CODES/mt19937ar.c
 * Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
 * All rights reserved.
 */
class T_DLLCLASS Random
{
public:
	Random(uint32_t seed = 5489UL);

	uint32_t next();

	double nextDouble();

	inline float nextFloat() { return float(nextDouble()); }

private:
	const static int32_t N = 624;

	uint32_t m_mt[N];
	int32_t m_mti;
};

}

