#ifndef traktor_flash_Polygon_H
#define traktor_flash_Polygon_H

#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace flash
	{

//@{
/*! \ingroup Flash */

struct Segment
{
	Vector2 v[2];
	uint16_t fillStyle0;
	uint16_t fillStyle1;
	uint16_t lineStyle;
};

struct Triangle
{
	Vector2 v[3];
	uint16_t fillStyle;
};

struct Line
{
	Vector2 v[2];
	uint16_t lineStyle;
};

//@}

	}
}

#endif	// traktor_flash_Polygon_H
