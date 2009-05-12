#ifndef traktor_render_VaryingUtils_H
#define traktor_render_VaryingUtils_H

#include "Core/Math/Vector4.h"
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

//@}

	}
}

#endif	// traktor_render_VaryingUtils_H
