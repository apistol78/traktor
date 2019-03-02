#pragma once

#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{

class SharpFallOff : public IFallOff
{
	T_RTTI_CLASS;

public:
	virtual float evaluate(float x, float y) const override final;
};

	}
}

