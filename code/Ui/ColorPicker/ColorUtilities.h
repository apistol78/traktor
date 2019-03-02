#pragma once

namespace traktor
{

class Color4f;

	namespace ui
	{

void RGBtoHSV(const Color4f& rgb, float outHsv[3]);

void HSVtoRGB(float hsv[3], Color4f& outRgb);

	}
}

