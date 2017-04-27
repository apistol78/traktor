/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Log2_H
#define traktor_Log2_H

#include "Core/Config.h"

namespace traktor
{

/*! \brief Get log-2 value.
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

/*! \brief Check if value is a log-2.
 * \ingroup Core
 */
bool T_FORCE_INLINE isLog2(int32_t v)
{
	return (1 << log2(v)) == v;
}

/*! \brief Get nearest log-2.
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

/*! \brief Get previous log-2.
 * \ingroup Core
 */
uint32_t T_FORCE_INLINE previousLog2(uint32_t num)
{
	uint32_t lg2 = nearestLog2(num);
	return lg2 > 1 ? (lg2 >> 1) : 1;
}

}

#endif	// traktor_Log2_H
