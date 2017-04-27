/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Random_H
#define traktor_Random_H

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
	
/*! \brief Random number generator.
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
	const static int32_t M = 397;
	const static uint32_t MATRIX_A = 0x9908b0dfUL;
	const static uint32_t UPPER_MASK = 0x80000000UL;
	const static uint32_t LOWER_MASK = 0x7fffffffUL;
	
	uint32_t m_mt[N];
	int32_t m_mti;
};
	
}

#endif	// traktor_Random_H
