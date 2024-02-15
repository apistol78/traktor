/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Core/Math/Color4f.h"
#include "Ui/ColorPicker/ColorUtilities.h"

namespace traktor
{
	namespace ui
	{

void RGBtoHSV(const Color4f& rgb, float outHsv[3])
{
	float T_MATH_ALIGN16 c[4];
	rgb.storeAligned(c);

	int32_t mn = 0;
	if (c[0] < c[1])
	{
		if (c[0] < c[2])
			mn = 0;
		else
			mn = 2;
	}
	else
	{
		if (c[1] < c[2])
			mn = 1;
		else
			mn = 2;
	}

	int32_t mx = 0;
	if (c[0] > c[1])
	{
		if (c[0] > c[2])
			mx = 0;
		else
			mx = 2;
	}
	else
	{
		if (c[1] > c[2])
			mx = 1;
		else
			mx = 2;
	}

	if (mn == mx)
		outHsv[0] = 0.0f;
	else if (mx == 0)
		outHsv[0] = 60.0f * (c[1] - c[2]) / (c[mx] - c[mn]);
	else if (mx == 1)
		outHsv[0] = 60.0f * (c[2] - c[0]) / (c[mx] - c[mn]) + 120.0f;
	else if (mx == 2)
		outHsv[0] = 60.0f * (c[0] - c[1]) / (c[mx] - c[mn]) + 240.0f;

	while (outHsv[0] < 0.0f)
		outHsv[0] += 360.0f;
	while (outHsv[0] >= 360.0f)
		outHsv[0] -= 360.0f;

	outHsv[2] = c[mx];

	if (c[mx] == 0)
		outHsv[1] = 0.0f;
	else
		outHsv[1] = 1.0f - c[mn] / c[mx];
}

void HSVtoRGB(float hsv[3], Color4f& outRgb)
{
	int32_t hi = int(hsv[0] / 60.0f) % 6;
	float f = hsv[0] / 60.0f - int(hsv[0] / 60.0f);
	float p = hsv[2] * (1.0f - hsv[1]);
	float q = hsv[2] * (1.0f - f * hsv[1]);
	float t = hsv[2] * (1.0f - (1.0f - f) * hsv[1]);

	float r = 0.0f, g = 0.0f, b = 0.0f;
	switch (hi)
	{
	case 0:
		r = hsv[2];
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		g = hsv[2];
		b = p;
		break;
	case 2:
		r = p;
		g = hsv[2];
		b = t;
		break;
	case 3:
		r = p;
		g = q;
		b = hsv[2];
		break;
	case 4:
		r = t;
		g = p;
		b = hsv[2];
		break;
	case 5:
		r = hsv[2];
		g = p;
		b = q;
		break;
	}

	outRgb = Color4f(r, g, b, 1.0f);
}

	}
}
