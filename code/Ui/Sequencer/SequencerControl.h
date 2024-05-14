/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
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

class ScrollBar;
class SequenceItem;

/*! Sequencer control.
 * \ingroup UI
 */
class T_DLLCLASS SequencerControl : public Widget
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t WsDragTrack = (WsUser << 1);

	enum GetSequenceFlags
	{
		GfDefault = 0,
		GfDescendants = 1,
		GfSelectedOnly = 2,
		GfExpandedOnly = 4
	};

	SequencerControl();

	bool create(Widget* parent, uint32_t style = WsDoubleBuffer | WsDragTrack);

	void setSeparator(int32_t separator);

	int32_t getSeparator() const;

	void setTimeScale(int32_t timeScale);

	int32_t getTimeScale() const;

	void setLength(int32_t length);

	int32_t getLength() const;

	void setCursor(int32_t time);

	int32_t getCursor() const;

	void setScrollOffset(const Point& scrollOffset);

	Point getScrollOffset() const;

	void addSequenceItem(SequenceItem* sequenceItem);

	void addSequenceItemBefore(SequenceItem* beforeItem, SequenceItem* sequenceItem);

	void removeSequenceItem(SequenceItem* sequenceItem);

	void removeAllSequenceItems();

	RefArray< SequenceItem > getSequenceItems(int32_t flags) const;

private:
	struct MouseTrackItem
	{
		Rect rc;
		Ref< SequenceItem > item;
	};

	Ref< ScrollBar > m_scrollBarV;
	Ref< ScrollBar > m_scrollBarH;
	RefArray< SequenceItem > m_sequenceItems;
	bool m_allowDragTracks;
	MouseTrackItem m_mouseTrackItem;
	int32_t m_separator;
	int32_t m_timeScale;
	int32_t m_length;
	int32_t m_cursor;
	Point m_startPosition;
	int32_t m_moveTrack;
	int32_t m_dropIndex;

	void updateScrollBars();

	void eventSize(SizeEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventPaint(PaintEvent* event);

	void eventScroll(ScrollEvent* event);
};

	}
}

