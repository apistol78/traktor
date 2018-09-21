/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_ColorUtilities_H
#define traktor_ui_custom_ColorUtilities_H

namespace traktor
{

class Color4f;

	namespace ui
	{
		namespace custom
		{

void RGBtoHSV(const Color4f& rgb, float outHsv[3]);

void HSVtoRGB(float hsv[3], Color4f& outRgb);

		}
	}
}

#endif	// traktor_ui_custom_ColorUtilities_H
