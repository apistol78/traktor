/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_TypesSw_H
#define traktor_render_TypesSw_H

#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace render
	{

inline uint32_t toARGB(const Vector4& v)
{
	int32_t r = int32_t(v.x() * 255.0f) & 255;
	int32_t g = int32_t(v.y() * 255.0f) & 255;
	int32_t b = int32_t(v.z() * 255.0f) & 255;
	int32_t a = int32_t(v.w() * 255.0f) & 255;
	return
		(a << 24) |
		(r << 16) |
		(g << 8) |
		(b);
}

inline Vector4 fromARGB(uint32_t v)
{
	return Vector4(
		((v & 0x00ff0000) >> 16) / 255.0f,
		((v & 0x0000ff00) >>  8) / 255.0f,
		((v & 0x000000ff) >>  0) / 255.0f,
		((v & 0xff000000) >> 24) / 255.0f
	);
}

	}
}

#endif	// traktor_render_TypesSw_H
