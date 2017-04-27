/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_MathCompare_H
#define traktor_MathCompare_H

#include "Core/Math/Vector4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Const.h"

namespace traktor
{

inline bool fuzzyEqual(float a, float b)
{
	return std::fabs(a - b) <= FUZZY_EPSILON;
}

inline bool compareVectorEqual(const Vector4& lh, const Vector4& rh)
{
	for (int i = 0; i < 4; ++i)
	{
		if (!fuzzyEqual(lh[i], rh[i]))
			return false;
	}
	return true;
}

inline bool compareQuaternionEqual(const Quaternion& lh, const Quaternion& rh)
{
	float d = dot4(Vector4(lh.e), Vector4(rh.e));
	return std::fabs(d) >= 1.0f - FUZZY_EPSILON;
}

inline bool compareMatrixEqual(const Matrix44& lh, const Matrix44& rh)
{
	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			if (!fuzzyEqual(lh(r, c), rh(r, c)))
				return false;
		}
	}
	return true;
}

inline bool compareTransformEqual(const Transform& lh, const Transform& rh)
{
	for (int i = 0; i < 4; ++i)
	{
		if (!fuzzyEqual(lh.translation()[i], rh.translation()[i]))
			return false;
		if (!fuzzyEqual(lh.rotation().e[i], rh.rotation().e[i]))
			return false;
	}
	return true;
}

}

#endif	// traktor_MathCompare_H
