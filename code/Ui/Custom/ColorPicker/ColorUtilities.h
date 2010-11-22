#ifndef traktor_ui_custom_ColorUtilities_H
#define traktor_ui_custom_ColorUtilities_H

namespace traktor
{

class Color4ub;

	namespace ui
	{
		namespace custom
		{

void RGBtoHSV(const Color4ub& rgb, float outHsv[3]);

void HSVtoRGB(float hsv[3], Color4ub& outRgb);

		}
	}
}

#endif	// traktor_ui_custom_ColorUtilities_H
