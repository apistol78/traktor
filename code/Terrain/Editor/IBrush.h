#ifndef traktor_terrain_IBrush_H
#define traktor_terrain_IBrush_H

#include "Core/Object.h"

namespace traktor
{
	namespace terrain
	{

class IFallOff;

class IBrush : public Object
{
	T_RTTI_CLASS;

public:
	virtual void begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength) = 0;

	virtual void apply(int32_t x, int32_t y) = 0;

	virtual void end(int32_t x, int32_t y) = 0;
};

	}
}

#endif	// traktor_terrain_IBrush_H
