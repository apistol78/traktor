/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_ScrollBar_H
#define traktor_ui_custom_ScrollBar_H

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
		namespace custom
		{

/*! \brief Scroll bar.
 * \ingroup UIC
 */
class T_DLLCLASS ScrollBar : public Widget
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsHorizontal = 0,
		WsVertical = WsUser
	};

	ScrollBar();

	bool create(Widget* parent, int32_t style = WsHorizontal);

	void setRange(int32_t range);

	int32_t getRange() const;

	void setPage(int32_t page);

	int32_t getPage() const;

	void setPosition(int32_t position);

	int32_t getPosition() const;

	virtual Size getPreferedSize() const T_OVERRIDE;

private:
	bool m_vertical;
	int32_t m_range;
	int32_t m_page;
	int32_t m_position;
	int32_t m_trackOffset;

	void eventMouseButtonDown(MouseButtonDownEvent* event);

	void eventMouseButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ScrollBar_H
