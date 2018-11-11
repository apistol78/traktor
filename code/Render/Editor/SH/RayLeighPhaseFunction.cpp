/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Editor/SH/RayLeighPhaseFunction.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"RayLeighPhaseFunction", RayLeighPhaseFunction, SHFunction)

float RayLeighPhaseFunction::evaluate(float phi, float theta, const Vector4& unit) const
{
	const Vector4 c_viewDirection(0.0f, 0.0f, 1.0f, 0.0f);
	float a = dot3(c_viewDirection, unit) * Scalar(2.0f);
	//return 1.0f - (a * a + 1.0f) * 3.0f / 4.0f;
	return max(a, 0.0f);
}
		
	}
}
