/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include "Ui/Custom/ColorPicker/ColorUtilities.h"
#include "Core/Math/Color4ub.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

void RGBtoHSV(const Color4ub& rgb, float outHsv[3])
{
	float c[] =
	{
		rgb.r / 255.0f,
		rgb.g / 255.0f,
		rgb.b / 255.0f
	};

	int mn = 0;
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

	int mx = 0;
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

	if (mx == 0)
		outHsv[1] = 0.0f;
	else
		outHsv[1] = 1.0f - c[mn] / c[mx];
}

void HSVtoRGB(float hsv[3], Color4ub& outRgb)
{
	int hi = int(hsv[0] / 60.0f) % 6;
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

	outRgb.r = uint8_t(r * 255.0f);
	outRgb.g = uint8_t(g * 255.0f);
	outRgb.b = uint8_t(b * 255.0f);
}

		}
	}
}