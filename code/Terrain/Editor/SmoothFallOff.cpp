#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Terrain/Editor/SmoothFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.SmoothFallOff", SmoothFallOff, IFallOff)

float SmoothFallOff::evaluate(float x, float y) const
{
	float d = x * x + y * y;
	if (d >= 1.0f)
		return 0.0f;

	float v = 1.0f - sqrtf(d);
	return clamp(sinf(v * PI / 4.0f), 0.0f, 1.0f);
}

	}
}
