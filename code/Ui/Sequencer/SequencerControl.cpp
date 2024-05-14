/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include <stack>
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/ScrollBar.h"
#include "Ui/Sequencer/CursorMoveEvent.h"
#include "Ui/Sequencer/SequencerControl.h"
#include "Ui/Sequencer/SequenceGroup.h"
#include "Ui/Sequencer/SequenceItem.h"
#include "Ui/Sequencer/SequenceMovedEvent.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_preferedHeightMargin = 2_ut;
const Unit c_sequenceHeight = 25_ut;
const int c_endWidth = 200;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SequencerControl", SequencerControl, Widget)

SequencerControl::SequencerControl()
:	m_allowDragTracks(false)
,	m_separator(0)
,	m_timeScale(8)
,	m_length(5000)
,	m_cursor(0)
,	m_moveTrack(-1)
,	m_dropIndex(-1)
{
}

bool SequencerControl::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style))
		return false;

	m_scrollBarV = new ScrollBar();
	if (!m_scrollBarV->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBarV->addEventHandler< ScrollEvent >(this, &SequencerControl::eventScroll);

	m_scrollBarH = new ScrollBar();
	if (!m_scrollBarH->create(this, ScrollBar::WsHorizontal))
		return false;

	m_scrollBarH->addEventHandler< ScrollEvent >(this, &SequencerControl::eventScroll);

	addEventHandler< SizeEvent >(this, &SequencerControl::eventSize);
	addEventHandler< MouseButtonDownEvent >(this, &SequencerControl::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &SequencerControl::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &SequencerControl::eventMouseMove);
	addEventHandler< MouseWheelEvent >(this, &SequencerControl::eventMouseWheel);
	addEventHandler< PaintEvent >(this, &SequencerControl::eventPaint);

	m_allowDragTracks = bool((style & WsDragTrack) == WsDragTrack);
	m_separator = pixel(200_ut);
	return true;
}

void SequencerControl::setSeparator(int32_t separator)
{
	m_separator = separator;
	updateScrollBars();
}

int32_t SequencerControl::getSeparator() const
{
	return m_separator;
}

void SequencerControl::setTimeScale(int32_t timeScale)
{
	m_timeScale = timeScale;
	updateScrollBars();
}

int32_t SequencerControl::getTimeScale() const
{
	return m_timeScale;
}

void SequencerControl::setLength(int32_t length)
{
	m_length = length;
	updateScrollBars();
}

int32_t SequencerControl::getLength() const
{
	return m_length;
}

void SequencerControl::setCursor(int32_t cursor)
{
	m_cursor = cursor;
}

int32_t SequencerControl::getCursor() const
{
	return m_cursor;
}

void SequencerControl::setScrollOffset(const Point& scrollOffset)
{
	m_scrollBarH->setPosition(scrollOffset.x);
	m_scrollBarV->setPosition(scrollOffset.y);
}

Point SequencerControl::getScrollOffset() const
{
	return Point(
		m_scrollBarH->getPosition(),
		m_scrollBarV->getPosition()
	);
}

void SequencerControl::addSequenceItem(SequenceItem* sequenceItem)
{
	m_sequenceItems.push_back(sequenceItem);
	updateScrollBars();
}

void SequencerControl::addSequenceItemBefore(SequenceItem* beforeItem, SequenceItem* sequenceItem)
{
	RefArray< SequenceItem >::iterator i = std::find(m_sequenceItems.begin(), m_sequenceItems.end(), beforeItem);
	T_ASSERT(i != m_sequenceItems.end());
	m_sequenceItems.insert(i, sequenceItem);
	updateScrollBars();
}

void SequencerControl::removeSequenceItem(SequenceItem* sequenceItem)
{
	if (sequenceItem->getParentItem())
		sequenceItem->getParentItem()->removeChildItem(sequenceItem);
	else
		m_sequenceItems.remove(sequenceItem);

	updateScrollBars();
}

void SequencerControl::removeAllSequenceItems()
{
	m_sequenceItems.clear();
	updateScrollBars();
}

RefArray< SequenceItem > SequencerControl::getSequenceItems(int32_t flags) const
{
	typedef std::pair< RefArray< SequenceItem >::const_iterator, RefArray< SequenceItem >::const_iterator > range_t;

	RefArray< SequenceItem > sequenceItems;

	std::stack< range_t > stack;
	stack.push(std::make_pair(m_sequenceItems.begin(), m_sequenceItems.end()));

	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			SequenceItem* item = *r.first++;

			if (flags & GfSelectedOnly)
			{
				if (item->isSelected())
					sequenceItems.push_back(item);
			}
			else
				sequenceItems.push_back(item);

			if (flags & GfDescendants)
			{
				if (is_a< SequenceGroup >(item) && (flags & GfExpandedOnly) != 0)
				{
					if (static_cast< SequenceGroup* >(item)->isCollapsed())
						continue;
				}

				RefArray< SequenceItem >& childItems = item->getChildItems();
				if (!childItems.empty())
				{
					stack.push(std::make_pair(
						childItems.begin(),
						childItems.end()
					));
				}
			}
		}
		else
			stack.pop();
	}

	return sequenceItems;
}

void SequencerControl::eventSize(SizeEvent* event)
{
	event->consume();

	Rect rc = getInnerRect();

	int32_t scrollWidth = m_scrollBarV->getPreferredSize(rc.getSize()).cx;
	int32_t scrollHeight = m_scrollBarH->getPreferredSize(rc.getSize()).cy;

	m_scrollBarV->setRect(Rect(
		rc.right - scrollWidth,
		rc.top,
		rc.right,
		rc.bottom - scrollHeight
	));

	m_scrollBarH->setRect(Rect(
		rc.left + m_separator,
		rc.bottom - scrollHeight,
		rc.right,
		rc.bottom
	));

	updateScrollBars();
}

void SequencerControl::updateScrollBars()
{
	int32_t sequenceHeight = pixel(c_sequenceHeight);

	// Get all items, including descendants.
	RefArray< SequenceItem > sequenceItems = getSequenceItems(GfDescendants | GfExpandedOnly);

	Size sequences(
		m_separator + m_length / m_timeScale + c_endWidth,
		int32_t(sequenceItems.size() * sequenceHeight) + 1
	);

	Rect rc = getInnerRect();

	int32_t scrollWidth = m_scrollBarV->getPreferredSize(rc.getSize()).cx;
	int32_t scrollHeight = m_scrollBarH->getPreferredSize(rc.getSize()).cy;

	int32_t overflowV = std::max< int32_t >(0, sequences.cy - rc.getHeight() + scrollHeight);
	m_scrollBarV->setRange(overflowV);
	m_scrollBarV->setEnable(overflowV > 0);
	m_scrollBarV->setPage(sequenceHeight);

	int32_t overflowH = std::max< int32_t >(0, sequences.cx - rc.getWidth() + scrollWidth);
	m_scrollBarH->setRange(overflowH);
	m_scrollBarH->setEnable(overflowH > 0);
	m_scrollBarH->setPage(100);
}

void SequencerControl::eventButtonDown(MouseButtonDownEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	int32_t sequenceHeight = pixel(c_sequenceHeight);
	Point position = event->getPosition();
	Rect rc = getInnerRect();

	// Grab focus, need it to be able to get key events.
	setFocus();

	// Get all items, including descendants.
	RefArray< SequenceItem > sequenceItems = getSequenceItems(GfDescendants | GfExpandedOnly);

	// Update only selection in left column.
	if (position.x < rc.left + m_separator)
	{
		bool selectionModified = false;

		// If not shift is down we de-select all items.
		if (!(event->getKeyState() & KsShift))
		{
			for (auto sequenceItem : sequenceItems)
				selectionModified |= sequenceItem->setSelected(false);
		}

		// Ensure sequence is selected.
		int32_t sequenceId = (position.y + m_scrollBarV->getPosition()) / sequenceHeight;
		if (sequenceId >= 0 && sequenceId < (int32_t)sequenceItems.size())
		{
			auto it = sequenceItems.begin();
			std::advance(it, sequenceId);
			selectionModified |= (*it)->setSelected(true);
		}

		// Issue selection change event.
		if (selectionModified)
		{
			SelectionChangeEvent selectionChangeEvent(this);
			raiseEvent(&selectionChangeEvent);
		}
	}

	// Issue local mouse down event on sequence item.
	int32_t sequenceId = (position.y + m_scrollBarV->getPosition()) / sequenceHeight;
	if (sequenceId >= 0 && sequenceId < (int32_t)sequenceItems.size())
	{
		auto it = sequenceItems.begin();
		std::advance(it, sequenceId);

		m_mouseTrackItem.rc = Rect(rc.left, 0, rc.right - m_scrollBarV->getPreferredSize(rc.getSize()).cx, sequenceHeight).offset(0, rc.top - m_scrollBarV->getPosition() + sequenceHeight * sequenceId);
		m_mouseTrackItem.item = *it;
		m_mouseTrackItem.item->mouseDown(
			this,
			Point(
				event->getPosition().x - m_mouseTrackItem.rc.left,
				event->getPosition().y - m_mouseTrackItem.rc.top
			),
			m_mouseTrackItem.rc,
			event->getButton(),
			m_separator,
			m_scrollBarH->getPosition()
		);
	}

	m_startPosition = position;

	if (position.x >= rc.left + m_separator)
	{
		m_moveTrack = 0;

		m_cursor = (position.x - m_separator + m_scrollBarH->getPosition()) * m_timeScale;
		m_cursor = std::max< int32_t >(m_cursor, 0);
		m_cursor = std::min< int32_t >(m_cursor, m_length);

		CursorMoveEvent cursorMoveEvent(this, m_cursor);
		raiseEvent(&cursorMoveEvent);
	}
	else if (m_allowDragTracks && m_mouseTrackItem.item)
	{
		m_moveTrack = 1;
		m_dropIndex = -1;
	}
	else
		m_moveTrack = -1;

	setCapture();
	update();

	event->consume();
}

void SequencerControl::eventButtonUp(MouseButtonUpEvent* event)
{
	if (!hasCapture())
		return;

	// Issue drop event.
	if (m_moveTrack == 2 && m_dropIndex >= 0)
	{
		T_ASSERT(m_mouseTrackItem.item);

		// Get all items, including descendants.
		RefArray< SequenceItem > sequenceItems = getSequenceItems(GfDescendants | GfExpandedOnly);

		if (m_dropIndex < int32_t(m_sequenceItems.size()))
		{
			SequenceItem* beforeItem = sequenceItems[m_dropIndex];
			if (beforeItem->getParentItem())
				beforeItem->getParentItem()->addChildItemBefore(beforeItem, m_mouseTrackItem.item);
			else
				addSequenceItemBefore(beforeItem, m_mouseTrackItem.item);
		}
		else
			addSequenceItem(m_mouseTrackItem.item);

		SequenceMovedEvent sequenceMovedEvent(this, m_mouseTrackItem.item, m_dropIndex);
		raiseEvent(&sequenceMovedEvent);
	}

	// Issue local mouse up event on tracked sequence item.
	if (m_mouseTrackItem.item)
	{
		m_mouseTrackItem.item->mouseUp(
			this,
			Point(
				event->getPosition().x - m_mouseTrackItem.rc.left,
				event->getPosition().y - m_mouseTrackItem.rc.top
			),
			m_mouseTrackItem.rc,
			event->getButton(),
			m_separator,
			m_scrollBarH->getPosition()
		);
		m_mouseTrackItem.item = nullptr;
	}

	m_moveTrack = 0;
	m_dropIndex = -1;

	releaseCapture();
	update();

	event->consume();
}

void SequencerControl::eventMouseMove(MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	event->consume();

	int32_t sequenceHeight = pixel(c_sequenceHeight);
	Point position = event->getPosition();

	// Check if begin moving.
	if (m_moveTrack == 1)
	{
		Size diff = m_startPosition - position;
		if (abs(diff.cx) > 2 || abs(diff.cy) > 2)
		{
			removeSequenceItem(m_mouseTrackItem.item);
			update();
			m_moveTrack = 2;
		}
		return;
	}

	// In case we're plain moving.
	if (m_moveTrack == 2)
	{
		// Get all items, including descendants.
		RefArray< SequenceItem > sequenceItems = getSequenceItems(GfDescendants | GfExpandedOnly);

		int32_t sequenceId = (position.y + sequenceHeight / 2 + m_scrollBarV->getPosition()) / sequenceHeight;
		if (sequenceId >= 0 && sequenceId < int(sequenceItems.size()))
			m_dropIndex = sequenceId;
		else
			m_dropIndex = int32_t(sequenceItems.size());

		update();
		return;
	}

	// Editing tickers.
	if (m_moveTrack == 0)
	{
		// Calculate current cursor display position.
		int32_t scrollOffsetX = m_scrollBarH->getPosition();

		int32_t cursor;
		cursor = (event->getPosition().x - m_separator + scrollOffsetX) * m_timeScale;
		cursor = std::max< int32_t >(cursor, 0);
		cursor = std::min< int32_t >(cursor, m_length);

		if (cursor == m_cursor)
			return;

		m_cursor = cursor;

		CursorMoveEvent cursorMoveEvent(this, m_cursor);
		raiseEvent(&cursorMoveEvent);

		update();

		// Notify track item mouse move.
		if (m_mouseTrackItem.item)
		{
			m_mouseTrackItem.item->mouseMove(
				this,
				Point(
					event->getPosition().x - m_mouseTrackItem.rc.left,
					event->getPosition().y - m_mouseTrackItem.rc.top
				),
				m_mouseTrackItem.rc,
				event->getButton(),
				m_separator,
				m_scrollBarH->getPosition()
			);
		}
	}
}

void SequencerControl::eventMouseWheel(MouseWheelEvent* event)
{
	int32_t wheel = event->getRotation();

	m_timeScale = clamp(m_timeScale + wheel, 1, 32);

	updateScrollBars();
	update();
}

void SequencerControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const StyleSheet* ss = getStyleSheet();
	const int32_t sequenceHeight = pixel(c_sequenceHeight);

	// Get all items, including descendants.
	RefArray< SequenceItem > sequenceItems = getSequenceItems(GfDescendants | GfExpandedOnly);

	// Get component sizes.
	const Rect rc = getInnerRect();
	const int32_t scrollWidth = m_scrollBarV->getPreferredSize(rc.getSize()).cx;
	const int32_t scrollHeight = m_scrollBarH->getPreferredSize(rc.getSize()).cy;

	// Get scroll offsets.
	const int32_t scrollOffsetX = m_scrollBarH->getPosition();
	const int32_t scrollOffsetY = m_scrollBarV->getPosition();

	// Clear background.
	canvas.setBackground(ss->getColor(this, L"control-background-color"));
	canvas.fillRect(Rect(rc.left + m_separator, rc.top, rc.right, rc.bottom));

	canvas.setBackground(ss->getColor(this, isEnable(true) ? L"background-color" : L"background-color-disabled"));
	canvas.fillRect(Rect(rc.left, rc.top, rc.left + m_separator, rc.bottom));

	// Right sequence edge.
	const int32_t end = std::min(m_separator + m_length / m_timeScale - scrollOffsetX, rc.right - scrollWidth);

	// Draw sequences.
	Rect rcSequence(
		rc.left,
		rc.top - scrollOffsetY,
		rc.right - scrollWidth,
		rc.top - scrollOffsetY + sequenceHeight
	);
	for (auto sequenceItem : sequenceItems)
	{
		canvas.setClipRect(Rect(
			rc.left,
			rc.top,
			end,
			rc.bottom - scrollHeight
		));
		sequenceItem->paint(this, canvas, rcSequence, m_separator, scrollOffsetX);
		rcSequence = rcSequence.offset(0, sequenceHeight);
	}

	canvas.resetClipRect();

	// Draw cursor.
	const int32_t x = m_separator + m_cursor / m_timeScale - scrollOffsetX;
	if (x >= m_separator && x < rc.right)
	{
		canvas.setForeground(ss->getColor(this, L"cursor-color"));
		canvas.drawLine(x, rc.top, x, rc.bottom - scrollHeight - 1);
	}

	// Draw drop position.
	if (m_dropIndex >= 0)
	{
		const int32_t y = rc.top - scrollOffsetY + m_dropIndex * sequenceHeight;
		canvas.setForeground(ss->getColor(this, L"drop-line-color"));
		canvas.drawLine(rc.left, y - 1, rc.right, y - 1);
		canvas.drawLine(rc.left, y, rc.right, y);
		canvas.drawLine(rc.left, y + 1, rc.right, y + 1);
	}

	// Draw time information.
	const int32_t infoHeight = getFontMetric().getHeight() + pixel(c_preferedHeightMargin) * 2;
	const Rect rcTime(
		rc.left,
		rc.bottom - infoHeight,
		rc.left + m_separator,
		rc.bottom
	);

	canvas.setBackground(ss->getColor(this, L"info-background-color"));
	canvas.fillRect(rcTime);

	const std::wstring ws = str(L"%d ms", m_cursor);
	const Size ext = canvas.getFontMetric().getExtent(ws);

	canvas.setForeground(ss->getColor(this, L"info-color"));
	canvas.drawText(
		Point(
			rcTime.left + pixel(8_ut),
			rcTime.top + (rcTime.getHeight() - ext.cy) / 2
		),
		ws
	);

	event->consume();
}

void SequencerControl::eventScroll(ScrollEvent* event)
{
	update();
	event->consume();
}

}
