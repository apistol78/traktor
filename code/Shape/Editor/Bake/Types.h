/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace shape
	{

class IProbe;

struct Surface
{
	int32_t count;
	Vector4 points[16];
	Vector2 texCoords[16];
	Vector4 normals[16];
	Vector4 normal;
	Color4f color;
	Scalar emissive;
	Scalar translucency;
	AlignedVector< uint32_t > shared[16];

	Surface()
	:	count(0)
	,	emissive(0.0f)
	,	translucency(0.0f)
	{
	}
};

struct Light
{
	enum LightType
	{
		LtDirectional = 0,
		LtPoint = 1,
		LtSpot = 2
	};

	enum LightMask
	{
		LmDirect = 0x01,
		LmIndirect = 0x02
	};

	LightType type;
	Vector4 position;
	Vector4 direction;
	Color4f color;
	Scalar range;
	Scalar radius;
	int32_t surface;
	uint8_t mask;

	Light()
	:	type(LtDirectional)
	,	position(Vector4::origo())
	,	direction(Vector4::zero())
	,	color(0.0f, 0.0f, 0.0f, 1.0f)
	,	range(0.0f)
	,	radius(0.0f)
	,	surface(0)
	,	mask(0)
	{
	}
};

	}
}
