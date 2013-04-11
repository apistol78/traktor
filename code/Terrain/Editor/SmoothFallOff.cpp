#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Terrain/Editor/SmoothFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.SmoothFallOff", SmoothFallOff, IFallOff)

float SmoothFallOff::evaluate(float x) const
{
	return clamp(sinf(x * PI / 4.0f), 0.0f, 1.0f);
}

	}
}
