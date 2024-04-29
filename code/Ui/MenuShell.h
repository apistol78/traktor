/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

namespace traktor::ui
{

class MenuItem;
class ScrollBar;

/*! Menu shell
 * \ingroup UI
 */
class T_DLLCLASS MenuShell : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, int32_t maxItems = -1);

	virtual void destroy() override;

	void add(MenuItem* item);

	MenuItem* getItem(const Point& at) const;

	bool getItemRect(const MenuItem* item, Rect& outItemRect) const;

	virtual Size getMinimumSize() const override final;

	virtual Size getPreferredSize(const Size& hint) const override final;

private:
	int32_t m_maxItems = -1;
	RefArray< MenuItem > m_items;
	Ref< MenuItem > m_activeItem;
	Ref< ScrollBar > m_scrollBar;
	Ref< MenuItem > m_trackItem;
	Ref< Widget > m_trackSubMenu;
	Ref< IEventHandler > m_eventHandlerButtonDown;
	Ref< IEventHandler > m_eventHandlerButtonUp;

	void eventMouseMove(MouseMoveEvent* event);

	void eventGlobalButtonDown(MouseButtonDownEvent* event);

	void eventGlobalButtonUp(MouseButtonUpEvent* event);

	void eventPaint(PaintEvent* e);

	void eventSize(SizeEvent* e);

	void eventScroll(ScrollEvent* e);
};

}
