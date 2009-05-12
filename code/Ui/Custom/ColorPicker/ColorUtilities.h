#ifndef traktor_ui_custom_ColorUtilities_H
#define traktor_ui_custom_ColorUtilities_H

namespace traktor
{

class Color;

	namespace ui
	{
		namespace custom
		{

void RGBtoHSV(const Color& rgb, float outHsv[3]);

void HSVtoRGB(float hsv[3], Color& outRgb);

		}
	}
}

#endif	// traktor_ui_custom_ColorUtilities_H
