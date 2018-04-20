/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <stack>
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/HierarchicalState.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/ScrollBar.h"
#include "Ui/Custom/PropertyList/PropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Custom/PropertyList/PropertySelectionChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

enum Modes
{
	MdNone,
	MdMoveSeparator
};

const int c_columnsHeight = 25;
const int c_propertyItemHeight = 20;
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

	std::wstring path = buildPath(item);
	outState->addState(path, item->isExpanded(), item->isSelected());

	const RefArray< PropertyItem >& children = item->getChildItems();
	for (RefArray< PropertyItem >::const_iterator i = children.begin(); i != children.end(); ++i)
		recursiveCaptureState(*i, outState);
}

void recursiveApplyState(PropertyItem* item, const HierarchicalState* state)
{
	if (!item)
		return;

	RefArray< PropertyItem >& children = item->getChildItems();
	for (RefArray< PropertyItem >::iterator i = children.begin(); i != children.end(); ++i)
		recursiveApplyState(*i, state);

	std::wstring path = buildPath(item);

	if (state->getExpanded(path))
		item->expand();
	else
		item->collapse();

	if (state->getSelected(path))
		item->setSelected(true);
	else
		item->setSelected(false);
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.PropertyList", PropertyList, Widget)

PropertyList::PropertyList()
:	m_guidResolver(0)
,	m_separator(dpi96(80))
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

	addEventHandler< MouseButtonDownEvent >(this, &PropertyList::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &PropertyList::eventButtonUp);
	addEventHandler< MouseDoubleClickEvent >(this, &PropertyList::eventDoubleClick);
	addEventHandler< MouseMoveEvent >(this, &PropertyList::eventMouseMove);
	addEventHandler< MouseWheelEvent >(this, &PropertyList::eventMouseWheel);
	addEventHandler< SizeEvent >(this, &PropertyList::eventSize);
	addEventHandler< PaintEvent >(this, &PropertyList::eventPaint);

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

	for (RefArray< PropertyItem >::iterator i = propertyItems.begin(); i != propertyItems.end(); ++i)
	{
		(*i)->destroyInPlaceControls();
		(*i)->setPropertyList(0);
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

			if (flags & GfSelectedOnly)
			{
				if (item->isSelected())
					propertyItems.push_back(item);
			}
			else
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

void PropertyList::setSeparator(int separator)
{
	m_separator = separator;
}

int PropertyList::getSeparator() const
{
	return m_separator;
}

void PropertyList::setColumnName(int column, const std::wstring& name)
{
	T_ASSERT (column == 0 || column == 1);
	m_columnNames[column] = name;
}

Ref< PropertyItem > PropertyList::getPropertyItemFromPosition(const Point& position)
{
	int32_t scrollBarOffset = m_scrollBar->getPosition() * dpi96(c_propertyItemHeight);

	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly);

	int32_t y = position.y;
	if (m_columnHeader)
	{
		y -= dpi96(c_columnsHeight);
		if (y < 0)
			return 0;
	}

	int32_t id = (y + scrollBarOffset) / dpi96(c_propertyItemHeight);
	for (RefArray< PropertyItem >::iterator i = propertyItems.begin(); i != propertyItems.end(); ++i)
	{
		if (id-- <= 0)
			return *i;
	}

	return 0;
}

bool PropertyList::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	if (!m_guidResolver)
		return false;

	return m_guidResolver->resolvePropertyGuid(guid, resolved);
}

Ref< HierarchicalState > PropertyList::captureState() const
{
	Ref< HierarchicalState > state = new HierarchicalState();
	
	state->setScrollPosition(m_scrollBar->getPosition());
	for (RefArray< PropertyItem >::const_iterator i = m_propertyItems.begin(); i != m_propertyItems.end(); ++i)
		recursiveCaptureState(*i, state);

	return state;
}

void PropertyList::applyState(const HierarchicalState* state)
{
	for (RefArray< PropertyItem >::iterator i = m_propertyItems.begin(); i != m_propertyItems.end(); ++i)
		recursiveApplyState(*i, state);

	updateScrollBar();
	m_scrollBar->setPosition(state->getScrollPosition());
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

Size PropertyList::getPreferedSize() const
{
	return Size(256, 256);
}

void PropertyList::updateScrollBar()
{
	Rect rc = getInnerRect();

	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly);

	int32_t height = rc.getHeight();
	if (m_columnHeader)
		height -= dpi96(c_columnsHeight);

	int32_t itemCount = int(propertyItems.size());
	int32_t pageCount = height / dpi96(c_propertyItemHeight);

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
	int32_t scrollBarOffset = m_scrollBar->getPosition() * dpi96(c_propertyItemHeight);
	int32_t scrollBarWidth = m_scrollBar->isVisible(false) ? m_scrollBar->getPreferedSize().cx : 0;
	int32_t top = m_columnHeader ? dpi96(c_columnsHeight) : 0;

	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly);

	std::vector< WidgetRect > childRects;

	// Issue resize of in-place controls on expanded items.
	Rect rcInner = getInnerRect();
	Rect rcItem(
		rcInner.left, -scrollBarOffset + top,
		rcInner.right - scrollBarWidth, -scrollBarOffset + top + dpi96(c_propertyItemHeight) - 1
	);
	for (RefArray< PropertyItem >::iterator i = propertyItems.begin(); i != propertyItems.end(); ++i)
	{
		Rect rcValue (rcItem.left + m_separator + 1, rcItem.top, rcItem.right, rcItem.bottom);
		(*i)->resizeInPlaceControls(rcValue, childRects);

		rcItem = rcItem.offset(0, dpi96(c_propertyItemHeight));
	}

	// Move all children at once.
	setChildRects(childRects);
}

void PropertyList::eventScroll(ScrollEvent* event)
{
	placeItems();
	update();
	event->consume();
}

void PropertyList::eventButtonDown(MouseButtonDownEvent* event)
{
	Point p = event->getPosition();

	setFocus();

	m_mousePropertyItem = 0;
	if (p.x >= m_separator - dpi96(2) && p.x <= m_separator + dpi96(2))
	{
		m_mode = MdMoveSeparator;
		setCursor(CrSizeWE);
		setCapture();
		event->consume();
	}
	else
	{
		int32_t scrollBarOffset = m_scrollBar->getPosition() * dpi96(c_propertyItemHeight);

		int32_t y = event->getPosition().y;
		if (m_columnHeader)
		{
			if ((y -= dpi96(c_columnsHeight)) < 0)
				return;
		}

		RefArray< PropertyItem > propertyItems;
		getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly);

		int32_t id = (y + scrollBarOffset) / dpi96(c_propertyItemHeight);
		for (RefArray< PropertyItem >::iterator i = propertyItems.begin(); i != propertyItems.end(); ++i)
		{
			if (int(std::distance(propertyItems.begin(), i)) == id)
			{
				if (p.x >= m_separator + dpi96(2))
				{
					m_mousePropertyItem = *i;
					m_mousePropertyItem->mouseButtonDown(event);
				}
				else if (p.x >= (*i)->getDepth() * dpi96(8) && p.x <= (*i)->getDepth() * dpi96(8) + dpi96(12))
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

	if (m_mousePropertyItem)
	{
		m_mousePropertyItem->mouseButtonUp(event);
		m_mousePropertyItem = 0;
		event->consume();
	}

	if (hasCapture())
		releaseCapture();
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
	Point p = event->getPosition();

	setCursor(CrArrow);

	if (m_mode == MdMoveSeparator)
	{
		m_separator = p.x;

		setCursor(CrSizeWE);

		placeItems();
		update();

		event->consume();
	}
	else
	{
		if (p.x >= m_separator - dpi96(2) && p.x <= m_separator + dpi96(2))
		{
			setCursor(CrSizeWE);
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
	Rect rc = getInnerRect();

	int32_t scrollWidth = m_scrollBar->getPreferedSize().cx;
	int32_t top = m_columnHeader ? dpi96(c_columnsHeight) : 0;

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
	Rect rcInner = getInnerRect();

	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	int32_t scrollBarOffset = m_scrollBar->getPosition() * dpi96(c_propertyItemHeight);
	int32_t scrollBarWidth = m_scrollBar->isVisible(false) ? m_scrollBar->getPreferedSize().cx : 0;
	int32_t top = m_columnHeader ? dpi96(c_columnsHeight) : 0;

	// Clear widget background.
	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	// Draw columns.
	if (m_columnHeader)
	{
		canvas.setBackground(ss->getColor(this, L"header-background-color"));
		canvas.fillRect(Rect(rcInner.left, rcInner.top, rcInner.right, rcInner.top + dpi96(c_columnsHeight)));

		canvas.setForeground(ss->getColor(this, this->isEnable() ? L"color" : L"color-disabled"));
		canvas.drawText(
			Rect(
				rcInner.left + 2, rcInner.top,
				rcInner.left + m_separator - 2, rcInner.top + dpi96(c_columnsHeight)
			),
			m_columnNames[0],
			AnLeft,
			AnCenter
		);

		canvas.setForeground(ss->getColor(this, this->isEnable() ? L"color" : L"color-disabled"));
		canvas.drawText(
			Rect(
				rcInner.left + m_separator + 2, rcInner.top,
				rcInner.right, rcInner.top + dpi96(c_columnsHeight)
			),
			m_columnNames[1],
			AnLeft,
			AnCenter
		);
	}

	// Get visible items.
	RefArray< PropertyItem > propertyItems;
	getPropertyItems(propertyItems, GfDescendants | GfExpandedOnly);

	// Draw property items.
	Rect rcItem(
		rcInner.left, -scrollBarOffset + top,
		rcInner.right - scrollBarWidth, -scrollBarOffset + top + dpi96(c_propertyItemHeight) - 1
	);
	RefArray< PropertyItem >::iterator i = propertyItems.begin();
	while (rcItem.bottom < top && i != propertyItems.end())
	{
		rcItem = rcItem.offset(0, dpi96(c_propertyItemHeight));
		++i;
	}
	while (i != propertyItems.end())
	{
		Rect rcText(rcItem.left, rcItem.top, rcItem.left + m_separator, rcItem.bottom);
		Rect rcValue(rcItem.left + m_separator + 1, rcItem.top, rcItem.right, rcItem.bottom);
		
		// Draw item background.
		canvas.setForeground(ss->getColor(this, (*i)->isSelected() ? L"item-color-selected" : L"color"));
		(*i)->paintBackground(canvas, rcItem);

		// Draw item text and possible expand image.
		canvas.setClipRect(rcText);
		canvas.setForeground(ss->getColor(this, (*i)->isSelected() ? L"item-color-selected" : L"color"));
		(*i)->paintText(canvas, rcText);
		canvas.resetClipRect();

		// Draw item value.
		canvas.setClipRect(rcValue);
		canvas.setForeground(ss->getColor(this, (*i)->isSelected() ? L"item-color-selected" : L"color"));
		(*i)->paintValue(canvas, rcValue);
		canvas.resetClipRect();

		// Draw horizontal item separator.
		canvas.setForeground(ss->getColor(this, L"line-color"));
		canvas.drawLine(Point(rcItem.left, rcItem.bottom), Point(rcItem.right, rcItem.bottom));

		// Draw vertical item separator.
		canvas.drawLine(
			Point(rcItem.left + m_separator, rcItem.top),
			Point(rcItem.left + m_separator, rcItem.bottom)
		);

		rcItem = rcItem.offset(0, dpi96(c_propertyItemHeight));
		++i;
	}

	event->consume();
}

		}
	}
}
