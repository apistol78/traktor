#ifndef traktor_terrain_SharpFallOff_H
#define traktor_terrain_SharpFallOff_H

#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{

class SharpFallOff : public IFallOff
{
	T_RTTI_CLASS;

public:
	virtual float evaluate(float x, float y) const;
};

	}
}

#endif	// traktor_terrain_SharpFallOff_H
