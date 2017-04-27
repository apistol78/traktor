/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Half.h"

namespace traktor
{

half_t floatToHalf(float value)
{
	int i = *(int*)&value;

	int s =  (i >> 16) & 0x00008000;
	int e = ((i >> 23) & 0x000000ff) - (127 - 15);
	int m =   i        & 0x007fffff;

	if (e <= 0)
	{
		if (e < -10)
		{
			return 0;
		}
		m = (m | 0x00800000) >> (1 - e);
		return s | (m >> 13);
	}
	else if (e == 0xff - (127 - 15))
	{
		if (m == 0)
		{
			return s | 0x7c00;
		} 
		else
		{
			m >>= 13;
			return s | 0x7c00 | m | (m == 0);
		}
	}
	else
	{
		if (e > 30)
		{
			return s | 0x7c00;
		}
		return s | (e << 10) | (m >> 13);
	}
}

namespace
{

inline float cf(unsigned int v)
{
	return *(float*)&v;
}

}

float halfToFloat(half_t value)
{
	int s = (value >> 15) & 0x00000001;
	int e = (value >> 10) & 0x0000001f;
	int m =  value        & 0x000003ff;

	if (e == 0)
	{
		if (m == 0)
		{
			return cf(s << 31);
		}
		else
		{
			while (!(m & 0x00000400))
			{
				m <<= 1;
				e -=  1;
			}

			e += 1;
			m &= ~0x00000400;
		}
	}
	else if (e == 31)
	{
		if (m == 0)
		{
			return cf((s << 31) | 0x7f800000);
		}
		else
		{
			return cf((s << 31) | 0x7f800000 | (m << 13));
		}
	}

	e = e + (127 - 15);
	m = m << 13;

	return cf((s << 31) | (e << 23) | m);
}

}
