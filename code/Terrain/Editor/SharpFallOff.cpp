#include "Terrain/Editor/SharpFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.SharpFallOff", SharpFallOff, IFallOff)

float SharpFallOff::evaluate(float x, float y) const
{
	float d = x * x + y * y;
	if (d >= 1.0f)
		return 0.0f;
	else
		return 1.0f;
}

	}
}
