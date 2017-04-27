/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VaryingUtils_H
#define traktor_render_VaryingUtils_H

#include "Core/Math/Vector4.h"
#include "Core/Math/Float.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup SW */
//@{

typedef Vector4 varying_data_t [4 + 2 + 8];

inline uint32_t getVaryingOffset(DataUsage usage, uint32_t index)
{
	const uint32_t usageOffsets[] = { 0, 1, 2, 3, 4, 6 };
	const uint32_t usageIndices[] = { 1, 1, 1, 1, 2, 8 };
	T_ASSERT (index < sizeof_array(usageOffsets));
	T_ASSERT (index < usageIndices[usage]);
	return usageOffsets[usage] + index;
}

inline void interpolateVaryings(const varying_data_t& v1, const varying_data_t& v2, float b, varying_data_t& out)
{
	traktor::Scalar b1(b), b2(1.0f - b);
	for (uint32_t i = 0; i < sizeof_array(v1); ++i)
		out[i] = b2 * v1[i] + b1 * v2[i];
}

inline void copyVaryings(varying_data_t& dst, const varying_data_t& src)
{
	for (uint32_t i = 0; i < sizeof_array(dst); ++i)
		dst[i] = src[i];
}

inline void checkVaryings(const varying_data_t& v)
{
#if defined(_DEBUG)
	for (uint32_t i = 0; i < sizeof_array(v); ++i)
	{
		T_ASSERT (!(isNan(v[i].x()) || isInfinite(v[i].x())));
		T_ASSERT (!(isNan(v[i].y()) || isInfinite(v[i].y())));
		T_ASSERT (!(isNan(v[i].z()) || isInfinite(v[i].z())));
		T_ASSERT (!(isNan(v[i].w()) || isInfinite(v[i].w())));
	}
#endif
}

//@}

	}
}

#endif	// traktor_render_VaryingUtils_H
