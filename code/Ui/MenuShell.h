/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

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
	
class MenuItem;

/*! \brief Menu shell
 * \ingroup UI
 */
class T_DLLCLASS MenuShell : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent);

	void add(MenuItem* item);

	MenuItem* getItem(const Point& at) const;

	bool getItemRect(const MenuItem* item, Rect& outItemRect) const;

	virtual Size getMinimumSize() const T_OVERRIDE T_FINAL;

	virtual Size getPreferedSize() const T_OVERRIDE T_FINAL;

private:
	RefArray< MenuItem > m_items;
	Ref< MenuItem > m_trackItem;
	Ref< Widget > m_trackSubMenu;

	void eventMouseMove(MouseMoveEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventPaint(PaintEvent* e);
};

	}
}

