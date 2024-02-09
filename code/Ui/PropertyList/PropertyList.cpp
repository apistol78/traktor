/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <stack>
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/HierarchicalState.h"
#include "Ui/ScrollBar.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/PropertyList/PropertyItem.h"
#include "Ui/PropertyList/PropertyList.h"
#include "Ui/PropertyList/PropertySelectionChangeEvent.h"

namespace traktor::ui
{
	namespace
	{

enum Modes
{
	MdNone,
	MdMoveSeparator
};

const Unit c_columnsHeight = 25_ut;
const int c_wheelRotationFactor = 2;

std::wstring buildPath(const PropertyItem* item)
{
	std::wstring path = item->getText();

	const PropertyItem* parent = item->getParentItem();
	if (parent)
	{
		const RefArray< PropertyItem >& children = parent->getChildItems();

		RefArray< PropertyItem >::const_iterator it = std::find(children.begin(), children.end(), item);
		T_FATAL_ASSERT (it != children.end());

		path = buildPath(parent) + L"/" + path + L":" + toString(std::distance(children.begin(), it));
	}

	return path;
}

void recursiveCaptureState(const PropertyItem* item, HierarchicalState* outState)
{
	if (!item)
		return;

	const std::wstring path = buildPath(item);
	outState->addState(path, item->isExpanded(), item->isSelected());

	for (const auto childItem : item->getChildItems())
		recursiveCaptureState(childItem, outState);
}

void recursiveApplyState(PropertyItem* item, const HierarchicalState* state)
{
	if (!item)
		return;

	for (const auto childItem : item->getChildItems())
		recursiveApplyState(childItem, state);

	const std::wstring path = buildPath(item);

	if (state->getExpanded(path))
		item->expand();
	else
		item->collapse();

	if (state->getSelected(path))
		item->setSelected(true);
	else
		item->setSelected(false);
}

void recursiveExpand(PropertyItem* item)
{
	if (!item)
		return;

	for (const auto childItem : item->getChildItems())
		recursiveExpand(childItem);

	item->expand();
}

void recursiveCollapse(PropertyItem* item)
{
	if (!item)
		return;

	for (const auto childItem : item->getChildItems())
		recursiveCollapse(childItem);

	item->collapse();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PropertyList", PropertyList, Widget)

PropertyList::PropertyList()
:	m_guidResolver(0)
,	m_separator(0)
,	m_mode(MdNone)
,	m_columnHeader(true)
{
	m_columnNames[0] = L"Name";
	m_columnNames[1] = L"Value";
}

bool PropertyList::create(Widget* parent, int style, IPropertyGuidResolver* guidResolver)
{
	if (!Widget::create(parent, style))
		return false;

	m_scrollBar = new ScrollBar();
	if (!m_scrollBar->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBar->addEventHandler< ScrollEvent >(this, &PropertyList::eventScroll);
	m_scrollBar->setVisible(false);

	addEventHandler< KeyDownEvent >(this, &PropertyList::eventKeyDown);
	addEventHandler< MouseButtonDownEvent >(this, &PropertyList::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &PropertyList::eventButtonUp);
	addEventHandler< MouseDoubleClickEvent >(this, &PropertyList::eventDoubleClick);
	addEventHandler< MouseMoveEvent >(this, &PropertyList::eventMouseMove);
	addEventHandler< MouseWheelEvent >(this, &PropertyList::eventMouseWheel);
	addEventHandler< SizeEvent >(this, &PropertyList::eventSize);
	addEventHandler< PaintEvent >(this, &PropertyList::eventPaint);

	m_separator = 80_ut;
	m_propertyItemHeight = getFont().getSize() + 10_ut;
	m_columnHeader = bool((style & WsColumnHeader) == WsColumnHeader);
	m_guidResolver = guidResolver;

	return true;
}

void PropertyList::destroy()
{
	removeAllPropertyItems();
	Widget::destroy();
}

void PropertyList::addPropertyItem(PropertyItem* propertyItem)
{
	m_propertyItems.push_back(propertyItem);
	propertyItem->setPropertyList(this);
	propertyItem->createInPlaceControls(this);
}

void PropertyList::removePropertyItem(PropertyItem* propertyItem)
{
	propertyItem->destroyInPlaceControls();
	propertyItem->setPropertyList(0);
	m_propertyItems.remove(propertyItem);
}

void PropertyList::addPropertyItem(PropertyItem* parentPropertyItem, PropertyItem* propertyItem)
{
	parentPropertyItem->addChildItem(propertyItem);
	propertyItem->setPropertyList(this);
	if (parentPropertyItem->isExpanded())
		propertyItem->createInPlaceControls(this);
}

void PropertyList::removePropertyItem(PropertyItem* parentPropertyItem, PropertyItem* propertyItem)
{
	// Remove all child items first.
	RefArray< PropertyItem >& childPropertyItems = propertyItem->getChildItems();
	while (!childPropertyItems.empty())
		removePropertyItem(propertyItem, childPropertyItems.front());

	// Remove in-place controls and then remove from parent.
	propertyItem->destroyInPlaceControls();
	propertyItem->setPropertyList(0);
	parentPropertyItem->removeChildItem(propertyItem);
}

void PropertyList::removeAllPropertyItems()
{
	if (m_propertyItems.empty())
		return;

	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants);

	// Remove all items prior to destroying in-place controls as UI may issue a redraw for each
	// removed in-place control.
	m_propertyItems.clear();

	for (auto propertyItem : propertyItems)
	{
		propertyItem->destroyInPlaceControls();
		propertyItem->setPropertyList(nullptr);
	}
}

int PropertyList::getPropertyItems(RefArray< PropertyItem >& propertyItems, int flags)
{
	typedef std::pair< RefArray< PropertyItem >::iterator, RefArray< PropertyItem >::iterator > range_t;

	std::stack< range_t > stack;
	stack.push(std::make_pair(m_propertyItems.begin(), m_propertyItems.end()));

	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			PropertyItem* item = *r.first++;

			bool shouldAdd = true;

			if (flags & GfSelectedOnly)
			{
				if (!item->isSelected())
					shouldAdd = false;
			}

			if (flags & GfVisibleOnly)
			{
				if (!item->isVisible())
					shouldAdd = false;
			}

			if (shouldAdd)
				propertyItems.push_back(item);

			if (flags & GfDescendants)
			{
				if ((flags & GfExpandedOnly) != 0 && item->isCollapsed())
					continue;

				RefArray< PropertyItem >& childItems = item->getChildItems();
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

	return int(propertyItems.size());
}

void PropertyList::setSeparator(Unit separator)
{
	m_separator = separator;
}

Unit PropertyList::getSeparator() const
{
	return m_separator;
}

void PropertyList::setColumnName(int column, const std::wstring& name)
{
	T_ASSERT(column == 0 || column == 1);
	m_columnNames[column] = name;
}

Ref< PropertyItem > PropertyList::getPropertyItemFromPosition(const Point& position)
{
	const int32_t scrollBarOffset = m_scrollBar->getPosition() * pixel(m_propertyItemHeight);

	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly | GfVisibleOnly);

	int32_t y = position.y;
	if (m_columnHeader)
	{
		y -= pixel(c_columnsHeight);
		if (y < 0)
			return 0;
	}

	int32_t id = (y + scrollBarOffset) / pixel(m_propertyItemHeight);
	for (const auto item : propertyItems)
	{
		if (id-- <= 0)
			return item;
	}

	return 0;
}

bool PropertyList::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	if (m_guidResolver)
		return m_guidResolver->resolvePropertyGuid(guid, resolved);
	else
		return false;
}

Ref< HierarchicalState > PropertyList::captureState() const
{
	Ref< HierarchicalState > state = new HierarchicalState();
	state->setScrollPosition(m_scrollBar->getPosition());
	for (const auto item : m_propertyItems)
		recursiveCaptureState(item, state);
	return state;
}

void PropertyList::applyState(const HierarchicalState* state)
{
	for (const auto item : m_propertyItems)
		recursiveApplyState(item, state);
	updateScrollBar();
	m_scrollBar->setPosition(state->getScrollPosition());
}

void PropertyList::expandAll()
{
	for (const auto item : m_propertyItems)
		recursiveExpand(item);

	updateScrollBar();
}

void PropertyList::collapseAll()
{
	for (const auto item : m_propertyItems)
		recursiveCollapse(item);

	updateScrollBar();
}

bool PropertyList::copy()
{
	RefArray< PropertyItem > selectedItems;
	getPropertyItems(selectedItems, GfDescendants | GfSelectedOnly);
	if (selectedItems.size() == 1)
		return selectedItems[0]->copy();
	else
		return false;
}

bool PropertyList::paste()
{
	RefArray< PropertyItem > selectedItems;
	getPropertyItems(selectedItems, GfDescendants | GfSelectedOnly);
	if (selectedItems.size() == 1 && selectedItems[0]->paste())
	{
		update();
		return true;
	}
	else
		return false;
}

void PropertyList::update(const Rect* rc, bool immediate)
{
	updateScrollBar();
	placeItems();

	Widget::update(rc, immediate);
}

Size PropertyList::getMinimumSize() const
{
	return Size(256, 256);
}

Size PropertyList::getPreferredSize(const Size& hint) const
{
	return Size(256, 256);
}

IBitmap* PropertyList::getBitmap(const wchar_t* const name)
{
	auto it = m_bitmaps.find(name);
	if (it == m_bitmaps.end())
		m_bitmaps[name] = new ui::StyleBitmap(name);
	return m_bitmaps[name];
}

void PropertyList::updateScrollBar()
{
	const Rect rc = getInnerRect();

	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly | GfVisibleOnly);

	int32_t height = rc.getHeight();
	if (m_columnHeader)
		height -= pixel(c_columnsHeight);

	const int32_t itemCount = (int32_t)propertyItems.size();
	const int32_t pageCount = height / pixel(m_propertyItemHeight);

	int32_t position = m_scrollBar->getPosition();
	if (position > itemCount)
		position = itemCount;

	m_scrollBar->setRange(itemCount);
	m_scrollBar->setPage(pageCount);
	m_scrollBar->setVisible(itemCount > pageCount);
	m_scrollBar->setPosition(position);
}

void PropertyList::placeItems()
{
	const Rect rcInner = getInnerRect();

	const int32_t scrollBarOffset = m_scrollBar->getPosition() * pixel(m_propertyItemHeight);
	const int32_t scrollBarWidth = m_scrollBar->isVisible(false) ? m_scrollBar->getPreferredSize(rcInner.getSize()).cx : 0;
	const int32_t top = m_columnHeader ? pixel(c_columnsHeight) : 0;

	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly | GfVisibleOnly);

	std::vector< WidgetRect > childRects;

	// Issue resize of in-place controls on expanded items.
	Rect rcItem(
		rcInner.left, -scrollBarOffset + top,
		rcInner.right - scrollBarWidth, -scrollBarOffset + top + pixel(m_propertyItemHeight) - 1
	);
	for (auto item : propertyItems)
	{
		const Rect rcValue(rcItem.left + pixel(m_separator) + 1, rcItem.top, rcItem.right, rcItem.bottom);
		item->resizeInPlaceControls(rcValue, childRects);
		rcItem = rcItem.offset(0, pixel(m_propertyItemHeight));
	}

	// Move all children at once.
	if (!childRects.empty())
		setChildRects(&childRects[0], (uint32_t)childRects.size(), true);
}

void PropertyList::eventScroll(ScrollEvent* event)
{
	placeItems();
	update();
	event->consume();
}

void PropertyList::eventKeyDown(KeyDownEvent* event)
{
	RefArray< PropertyItem > items;
	getPropertyItems(items, PropertyList::GfDescendants | PropertyList::GfExpandedOnly | PropertyList::GfVisibleOnly);

	// Find index of selected item.
	int32_t current = -1;
	for (int32_t i = 0; i < items.size(); ++i)
	{
		if (items[i]->isSelected())
		{
			current = i;
			break;
		}
	}
	if (current < 0)
		return;

	for (auto item : items)
		item->setSelected(false);

	switch (event->getVirtualKey())
	{
	case VkLeft:
		if (items[current]->isExpanded())
		{
			items[current]->collapse();
			items[current]->setSelected(true);
		}
		else if (items[current]->getParentItem() != nullptr)
			items[current]->getParentItem()->setSelected(true);
		break;

	case VkRight:
		if (!items[current]->getChildItems().empty())
		{
			if (items[current]->isCollapsed())
			{
				items[current]->expand();
				items[current]->setSelected(true);
			}
			else
				items[current + 1]->setSelected(true);
		}
		break;

	case VkUp:
		if (current > 0)
			items[current - 1]->setSelected(true);
		else
			items[current]->setSelected(true);
		break;

	case VkDown:
		if (current < items.size() - 1)
			items[current + 1]->setSelected(true);
		else
			items[current]->setSelected(true);
		break;

	default:
		items[current]->setSelected(true);
		break;
	}

	update();
}

void PropertyList::eventButtonDown(MouseButtonDownEvent* event)
{
	const Point p = event->getPosition();

	setFocus();

	m_mousePropertyItem = 0;
	if (p.x >= pixel(m_separator - 2_ut) && p.x <= pixel(m_separator + 2_ut))
	{
		m_mode = MdMoveSeparator;
		setCursor(Cursor::SizeWE);
		setCapture();
		event->consume();
	}
	else
	{
		const int32_t scrollBarOffset = m_scrollBar->getPosition() * pixel(m_propertyItemHeight);

		int32_t y = event->getPosition().y;
		if (m_columnHeader)
		{
			if ((y -= pixel(c_columnsHeight)) < 0)
				return;
		}

		RefArray< PropertyItem > propertyItems;
		getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly | GfVisibleOnly);

		const int32_t id = (y + scrollBarOffset) / pixel(m_propertyItemHeight);
		for (RefArray< PropertyItem >::iterator i = propertyItems.begin(); i != propertyItems.end(); ++i)
		{
			if (int(std::distance(propertyItems.begin(), i)) == id)
			{
				if (p.x >= pixel(m_separator + 2_ut))
				{
					m_mousePropertyItem = *i;
					m_mousePropertyItem->mouseButtonDown(event);
				}
				else if (p.x >= (*i)->getDepth() * pixel(8_ut) && p.x <= (*i)->getDepth() * pixel(8_ut) + pixel(12_ut))
				{
					if ((*i)->isExpanded())
						(*i)->collapse();
					else
						(*i)->expand();

					updateScrollBar();
					placeItems();
				}

				if (!(*i)->isSelected())
				{
					(*i)->setSelected(true);

					PropertySelectionChangeEvent selectionChangeEvent(this, *i, id);
					raiseEvent(&selectionChangeEvent);
				}
			}
			else if ((*i)->isSelected())
			{
				(*i)->setSelected(false);

				PropertySelectionChangeEvent selectionChangeEvent(this, *i, id);
				raiseEvent(&selectionChangeEvent);
			}
		}

		if (m_mousePropertyItem)
		{
			setCapture();
			event->consume();
		}
	}

	update();
}

void PropertyList::eventButtonUp(MouseButtonUpEvent* event)
{
	m_mode = MdNone;

	if (hasCapture())
		releaseCapture();

	if (m_mousePropertyItem)
	{
		m_mousePropertyItem->mouseButtonUp(event);
		m_mousePropertyItem = 0;
		event->consume();
	}
}

void PropertyList::eventDoubleClick(MouseDoubleClickEvent* event)
{
	const Point& position = event->getPosition();

	Ref< PropertyItem > propertyItem = getPropertyItemFromPosition(position);
	if (propertyItem)
	{
		if (propertyItem->isExpanded())
			propertyItem->collapse();
		else
			propertyItem->expand();

		updateScrollBar();
		placeItems();

		event->consume();
	}

	update();
}

void PropertyList::eventMouseMove(MouseMoveEvent* event)
{
	const Point p = event->getPosition();

	setCursor(Cursor::Arrow);

	if (m_mode == MdMoveSeparator)
	{
		m_separator = unit(p.x);

		setCursor(Cursor::SizeWE);

		placeItems();
		update();

		event->consume();
	}
	else
	{
		if (p.x >= pixel(m_separator - 2_ut) && p.x <= pixel(m_separator + 2_ut))
		{
			setCursor(Cursor::SizeWE);
			event->consume();
		}
		else if (m_mousePropertyItem)
		{
			m_mousePropertyItem->mouseMove(event);
		}
	}
}

void PropertyList::eventMouseWheel(MouseWheelEvent* event)
{
	int32_t position = m_scrollBar->getPosition();
	position -= event->getRotation() * c_wheelRotationFactor;
	m_scrollBar->setPosition(position);

	placeItems();
	update();
}

void PropertyList::eventSize(SizeEvent* event)
{
	const Rect rc = getInnerRect();

	const int32_t scrollWidth = m_scrollBar->getPreferredSize(rc.getSize()).cx;
	const int32_t top = m_columnHeader ? pixel(c_columnsHeight) : 0;

	m_scrollBar->setRect(Rect(
		rc.right - scrollWidth,
		rc.top + top,
		rc.right,
		rc.bottom
	));

	updateScrollBar();
	placeItems();

	event->consume();
}

void PropertyList::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = getInnerRect();
	const StyleSheet* ss = getStyleSheet();
	const bool enabled = isEnable(true);

	const int32_t scrollBarOffset = m_scrollBar->getPosition() * pixel(m_propertyItemHeight);
	const int32_t scrollBarWidth = m_scrollBar->isVisible(false) ? m_scrollBar->getPreferredSize(rcInner.getSize()).cx : 0;
	const int32_t top = m_columnHeader ? pixel(c_columnsHeight) : 0;

	// Clear widget background.
	canvas.setBackground(ss->getColor(this, enabled ? L"background-color" : L"background-color-disabled"));
	canvas.fillRect(rcInner);

	// Draw columns.
	if (m_columnHeader)
	{
		canvas.setBackground(ss->getColor(this, L"header-background-color"));
		canvas.fillRect(Rect(rcInner.left, rcInner.top, rcInner.right, rcInner.top + pixel(c_columnsHeight)));

		canvas.setForeground(ss->getColor(this, enabled ? L"color" : L"color-disabled"));
		canvas.drawText(
			Rect(
				rcInner.left + 2, rcInner.top,
				rcInner.left + pixel(m_separator) - 2, rcInner.top + pixel(c_columnsHeight)
			),
			m_columnNames[0],
			AnLeft,
			AnCenter
		);

		canvas.setForeground(ss->getColor(this, enabled ? L"color" : L"color-disabled"));
		canvas.drawText(
			Rect(
				rcInner.left + pixel(m_separator) + 2, rcInner.top,
				rcInner.right, rcInner.top + pixel(c_columnsHeight)
			),
			m_columnNames[1],
			AnLeft,
			AnCenter
		);
	}

	// Get visible items.
	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly | GfVisibleOnly);

	// Draw property items.
	Rect rcItem(
		rcInner.left, -scrollBarOffset + top,
		rcInner.right - scrollBarWidth, -scrollBarOffset + top + pixel(m_propertyItemHeight) - 1
	);
	RefArray< PropertyItem >::iterator i = propertyItems.begin();
	while (rcItem.bottom < top && i != propertyItems.end())
	{
		rcItem = rcItem.offset(0, pixel(m_propertyItemHeight));
		++i;
	}
	while (rcItem.top < rcInner.bottom && i != propertyItems.end())
	{
		const Rect rcText(rcItem.left, rcItem.top, rcItem.left + pixel(m_separator), rcItem.bottom);
		const Rect rcValue(rcItem.left + pixel(m_separator) + 1, rcItem.top, rcItem.right, rcItem.bottom);

		// Draw item background.
		canvas.setForeground(ss->getColor((*i), (*i)->isSelected() ? L"color-selected" : L"color"));
		(*i)->paintBackground(canvas, rcItem);

		// Draw item text and possible expand image.
		canvas.setClipRect(rcText);
		canvas.setForeground(ss->getColor((*i), (*i)->isSelected() ? L"color-selected" : L"color"));
		(*i)->paintText(canvas, rcText);
		canvas.resetClipRect();

		// Draw item value.
		canvas.setClipRect(rcValue);
		canvas.setForeground(ss->getColor((*i), (*i)->isSelected() ? L"color-selected" : L"color"));
		(*i)->paintValue(this, canvas, rcValue);
		canvas.resetClipRect();

		// Draw horizontal item separator.
		canvas.setForeground(ss->getColor(this, L"line-color"));
		canvas.drawLine(Point(rcItem.left, rcItem.bottom), Point(rcItem.right, rcItem.bottom));

		// Draw vertical item separator.
		canvas.drawLine(
			Point(rcItem.left + pixel(m_separator), rcItem.top),
			Point(rcItem.left + pixel(m_separator), rcItem.bottom)
		);

		rcItem = rcItem.offset(0, pixel(m_propertyItemHeight));
		++i;
	}

	event->consume();
}

}
