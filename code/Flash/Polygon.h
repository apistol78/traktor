#ifndef traktor_flash_Polygon_H
#define traktor_flash_Polygon_H

#include "Flash/Vector2i.h"

namespace traktor
{
	namespace flash
	{

//@{
/*! \ingroup Flash */

struct Segment
{
	Vector2i v[2];
	Vector2i c;
	bool curve;
	uint16_t fillStyle0;
	uint16_t fillStyle1;
	uint16_t lineStyle;
};

enum TriangleType
{
	TcFill = 0,
	TcIn = 1,
	TcOut = 2
};

struct Triangle
{
	Vector2i v[3];
	TriangleType type;
	uint16_t fillStyle;
};

struct Line
{
	Vector2i v[2];
	uint16_t lineStyle;
};

//@}

	}
}

#endif	// traktor_flash_Polygon_H
