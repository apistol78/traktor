/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Random.h"

namespace traktor
{
	
Random::Random(uint32_t seed)
{
	m_mt[0] = seed & 0xffffffffUL;
	for (m_mti = 1; m_mti < N; ++m_mti)
	{
		m_mt[m_mti] = (1812433253UL * (m_mt[m_mti - 1] ^ (m_mt[m_mti - 1] >> 30)) + m_mti);
		m_mt[m_mti] &= 0xffffffffUL;
	}
}

uint32_t Random::next()
{
	uint32_t y;
	static uint32_t mag01[2] = { 0x0UL, MATRIX_A };

	if (m_mti >= N)
	{
		int kk;
		for (kk = 0; kk < N - M; ++kk)
		{
			y = (m_mt[kk] & UPPER_MASK) | (m_mt[kk + 1] & LOWER_MASK);
			m_mt[kk] = m_mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (; kk < N - 1; ++kk)
		{
			y = (m_mt[kk] & UPPER_MASK) | (m_mt[kk + 1] & LOWER_MASK);
			m_mt[kk] = m_mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (m_mt[N - 1] & UPPER_MASK) | (m_mt[0] &  LOWER_MASK);
		m_mt[N - 1] = m_mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		m_mti = 0;
	}

	y = m_mt[m_mti++];

	/* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return y;	
}

double Random::nextDouble()
{
	return next() * (1.0 / 4294967295.0);
}

}
