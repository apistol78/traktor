#include "Terrain/Editor/SharpFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.SharpFallOff", SharpFallOff, IFallOff)

float SharpFallOff::evaluate(float x) const
{
	return 1.0f;
}

	}
}
