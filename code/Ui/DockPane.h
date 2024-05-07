/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
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

class Canvas;
class Form;
class IBitmap;

/*! Docking pane.
 * \ingroup UI
 */
class T_DLLCLASS DockPane : public Object
{
	T_RTTI_CLASS;

public:
	enum Direction
	{
		DrNorth,
		DrSouth,
		DrWest,
		DrEast
	};

	explicit DockPane(Widget* owner, DockPane* parent);

	/*! Split pane into two sub panes.
	 * Pane to be split cannot contain a docked widget.
	 */
	void split(bool vertical, Unit split, Ref< DockPane >& outLeftPane, Ref< DockPane >& outRightPane);

	/*! Dock widget onto pane.
	 */
	void dock(Widget* widget);

	/*! Dock widget onto pane, if pane already contain a widget then split pane first.
	 */
	void dock(Widget* widget, Direction direction, Unit split);

	/*! Un-dock widget from pane.
	 */
	void undock(Widget* widget);

	/*! Detach pane, ie make it floating.
	 */
	void detach();

	void showTab(int32_t tab);


	void showWidget(Widget* widget);

	void hideWidget(Widget* widget);

	bool isWidgetVisible(const Widget* widget) const;


	DockPane* findWidgetPane(const Widget* widget);

	DockPane* getPaneFromPosition(const Point& position);

	DockPane* getSplitterFromPosition(const Point& position);

	bool hitGripper(const Point& position) const;

	bool hitGripperClose(const Point& position) const;

	bool hitSplitter(const Point& position) const;

	int32_t hitTab(const Point& position) const;

	void setSplitterPosition(const Point& position);

	bool isSplitter() const { return m_child[0] != nullptr && m_child[1] != nullptr; }

	const Rect& getPaneRect() const { return m_rect; }

	void setDetachable(bool detachable);

	bool isDetachable() const;

	void setStackable(bool stackable);

	bool isStackable() const;

	void setAlwaysVisible(bool alwaysVisible);

	bool isVisible() const;

private:
	friend class Dock;

	struct WidgetInfo
	{
		Ref< Widget > widget;
		int32_t tabMin;
		int32_t tabMax;
		int32_t tabDepth;
		bool visible;
	};

	Widget* m_owner = nullptr;
	DockPane* m_parent = nullptr;
	AlignedVector< WidgetInfo > m_widgets;
	Ref< IBitmap > m_bitmapClose;
	Ref< IBitmap > m_bitmapGripper;
	Ref< DockPane > m_child[2];
	Unit m_split = 0_ut;
	Unit m_gripperDim = 0_ut;
	Rect m_rect;
	bool m_detachable = true;
	bool m_stackable = false;
	bool m_alwaysVisible = false;
	bool m_vertical = false;

	void synchronizeVisibility();

	void update(const Rect& rect, AlignedVector< WidgetRect >& outWidgetRects);

	void draw(Canvas& canvas);
};

}
