/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_ToolBar_H
#define traktor_ui_custom_ToolBar_H

#include "Core/RefArray.h"
#include "Ui/Command.h"
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

class IBitmap;

		namespace custom
		{

class ToolBarItem;
class ToolTip;
class ToolTipEvent;

/*! \brief Tool bar control.
 * \ingroup UIC
 */
class T_DLLCLASS ToolBar : public Widget
{
	T_RTTI_CLASS;

public:
	enum WidgetStyles
	{
		WsUnderline = (WsUser)
	};

	ToolBar();

	bool create(Widget* parent, int style = WsNone);

	virtual void destroy() T_OVERRIDE;

	uint32_t addImage(IBitmap* image, uint32_t imageCount);

	uint32_t addItem(ToolBarItem* item);

	void setItem(uint32_t id, ToolBarItem* item);

	Ref< ToolBarItem > getItem(uint32_t id);

	Ref< ToolBarItem > getItem(const Point& at);

	virtual Size getPreferedSize() const T_OVERRIDE;

private:
	Ref< ToolTip > m_toolTip;
	int m_style;
	Ref< IBitmap > m_imageEnabled;
	Ref< IBitmap > m_imageDisabled;
	uint32_t m_imageWidth;
	uint32_t m_imageHeight;
	uint32_t m_imageCount;
	RefArray< ToolBarItem > m_items;
	Ref< ToolBarItem > m_trackItem;

	void eventMouseMove(MouseMoveEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventPaint(PaintEvent* event);

	void eventShowTip(ToolTipEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBar_H
