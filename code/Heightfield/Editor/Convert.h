#ifndef traktor_hf_Convert_H
#define traktor_hf_Convert_H

#include "Heightfield/HeightfieldTypes.h"

namespace traktor
{
	namespace hf
	{

inline int32_t unpackSignedHeight(height_t h)
{
	return (int32_t(h) - 32767) * 2;
}

inline height_t packSignedHeight(int32_t h)
{
	return height_t(int32_t(h) / 2 + 32767);
}

	}
}

#endif	// traktor_hf_Convert_H
