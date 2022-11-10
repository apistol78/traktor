/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace spark
	{

//@{
/*! \ingroup Spark */

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

