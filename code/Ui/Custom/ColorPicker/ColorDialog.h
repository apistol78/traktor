/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_ColorDialog_H
#define traktor_ui_custom_ColorDialog_H

#include "Ui/ConfigDialog.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Edit;

		namespace custom
		{

class ColorControl;
class ColorEvent;
class ColorGradientControl;
class ColorSliderControl;

struct ColorGradient;
struct AlphaGradient;

/*! \brief Color picker dialog.
 * \ingroup UIC
 */
class T_DLLCLASS ColorDialog : public ConfigDialog
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsAlpha = WsUser
	};

	bool create(Widget* parent, const std::wstring& text, int style = WsDefaultFixed, const Color4ub& initialColor = Color4ub(255, 255, 255, 255));

	Color4ub getColor() const;

private:
	Ref< ColorGradientControl > m_gradientControl;
	Ref< ColorSliderControl > m_sliderColorControl;
	Ref< ColorSliderControl > m_sliderAlphaControl;
	Ref< ColorControl > m_colorControl;
	Ref< Edit > m_editColor[4];
	Ref< ColorGradient > m_colorGradient;
	Ref< AlphaGradient > m_alphaGradient;
	Color4ub m_color;

	void updateControls();

	void eventGradientColorSelect(ColorEvent* event);

	void eventSliderColorSelect(ColorEvent* event);

	void eventSliderAlphaSelect(ColorEvent* event);

	void eventEditFocus(FocusEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ColorDialog_H
