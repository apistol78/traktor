/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Slider_H
#define traktor_ui_Slider_H

#include "Core/Math/Range.h"
#include "Ui/Widget.h"

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

/*! \brief Slider
 * \ingroup UI
 */
class T_DLLCLASS Slider : public Widget
{
	T_RTTI_CLASS;

public:
	Slider();

	bool create(Widget* parent, int32_t style = WsNone);

	void setRange(int32_t minValue, int32_t maxValue);

	void setValue(int32_t value);

	int32_t getValue() const;

	virtual Size getPreferedSize() const T_OVERRIDE;

private:
	Range< int32_t > m_range;
	int32_t m_value;
	bool m_drag;

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

#endif	// traktor_ui_Slider_H
