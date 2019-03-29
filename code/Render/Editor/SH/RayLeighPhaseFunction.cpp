#include "Render/Editor/SH/RayLeighPhaseFunction.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"RayLeighPhaseFunction", RayLeighPhaseFunction, SHFunction)

Vector4 RayLeighPhaseFunction::evaluate(float phi, float theta, const Vector4& unit) const
{
	const Vector4 c_viewDirection(0.0f, 0.0f, 1.0f, 0.0f);
	Scalar a = max(dot3(c_viewDirection, unit) * Scalar(2.0f), Scalar(0.0f));
	//return 1.0f - (a * a + 1.0f) * 3.0f / 4.0f;
	return Vector4(a, a, a, 0.0f);
}

	}
}
