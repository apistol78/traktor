#ifndef traktor_terrain_IFallOff_H
#define traktor_terrain_IFallOff_H

#include "Core/Object.h"

namespace traktor
{
	namespace terrain
	{

class IFallOff : public Object
{
	T_RTTI_CLASS;

public:
	virtual float evaluate(float x) const = 0;
};

	}
}

#endif	// traktor_terrain_IFallOff_H
