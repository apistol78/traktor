#ifndef traktor_terrain_IBrush_H
#define traktor_terrain_IBrush_H

#include "Core/Object.h"

namespace traktor
{

class Color4f;

	namespace terrain
	{

class IFallOff;

class IBrush : public Object
{
	T_RTTI_CLASS;

public:
	enum Mode
	{
		MdColor = 1,
		MdHeight = 2,
		MdCut = 4
	};

	virtual uint32_t begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color) = 0;

	virtual void apply(int32_t x, int32_t y) = 0;

	virtual void end(int32_t x, int32_t y) = 0;
};

	}
}

#endif	// traktor_terrain_IBrush_H
