/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_ColorGradientControl_H
#define traktor_ui_custom_ColorGradientControl_H

#include "Ui/Widget.h"

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

class Bitmap;

		namespace custom
		{

/*! \brief Color gradient control.
 * \ingroup UIC
 */
class T_DLLCLASS ColorGradientControl : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, int style, const Color4ub& color);

	virtual Size getPreferedSize() const T_OVERRIDE;

	void setColor(const Color4ub& color, bool updateCursor);

	Color4ub getColor() const;

private:
	float m_hue;
	Point m_cursor;
	Ref< Bitmap > m_gradientBitmap;

	void updateGradientImage();

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ColorGradientControl_H
