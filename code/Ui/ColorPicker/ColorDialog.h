/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4f.h"
#include "Ui/ConfigDialog.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class ColorControl;
class ColorEvent;
class ColorGradientControl;
class ColorSliderControl;
class Edit;

struct ColorGradient;
struct AlphaGradient;

/*! Color picker dialog.
 * \ingroup UI
 */
class T_DLLCLASS ColorDialog : public ConfigDialog
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsAlpha = WsUser,
		WsHDR = (WsUser << 1)
	};

	bool create(Widget* parent, const std::wstring& text, int32_t style = WsDefaultFixed, const Color4f& initialColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f));

	void setColor(const Color4f& color);

	Color4f getColor() const;

private:
	Ref< ColorGradientControl > m_gradientControl;
	Ref< ColorSliderControl > m_sliderColorControl;
	Ref< ColorSliderControl > m_sliderAlphaControl;
	Ref< ColorControl > m_colorControl;
	Ref< Edit > m_editColor[5];
	Ref< ColorGradient > m_colorGradient;
	Ref< AlphaGradient > m_alphaGradient;
	Color4f m_color;
	Scalar m_ev = 0.0_simd;

	void updateTextControls();

	void eventGradientColorSelect(ColorEvent* event);

	void eventSliderColorSelect(ColorEvent* event);

	void eventSliderAlphaSelect(ColorEvent* event);

	void eventEditFocus(FocusEvent* event);
};

	}
}
