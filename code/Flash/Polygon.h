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
	Vector2 c;
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
	Vector2 v[3];
	uint8_t type;	//!< \sa TriangleType
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
