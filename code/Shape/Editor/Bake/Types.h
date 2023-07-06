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

namespace traktor::shape
{

class IProbe;

struct Surface
{
	int32_t count = 0;
	Vector4 points[16];
	Vector2 texCoords[16];
	Vector4 normals[16];
	Vector4 normal;
	Color4f color;
	Scalar emissive = 0.0_simd;
	Scalar translucency = 0.0_simd;
	AlignedVector< uint32_t > shared[16];
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

	LightType type = LtDirectional;
	Vector4 position = Vector4::origo();
	Vector4 direction = Vector4::zero();
	Color4f color = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	Scalar range = 0.0_simd;
	Scalar radius = 0.0_simd;
	int32_t surface = 0;
	uint8_t mask = 0;
};

}
