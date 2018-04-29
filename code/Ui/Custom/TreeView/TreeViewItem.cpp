/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/IBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/TreeView/TreeView.h"
#include "Ui/Custom/TreeView/TreeViewDragEvent.h"
#include "Ui/Custom/TreeView/TreeViewItem.h"
#include "Ui/Custom/TreeView/TreeViewItemActivateEvent.h"
#include "Ui/Custom/TreeView/TreeViewItemStateChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

struct ItemSortPredicate
{
	bool operator () (const TreeViewItem* item1, const TreeViewItem* item2) const
	{
		return compareIgnoreCase(item1->getText(), item2->getText()) < 0;
	}
};

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.TreeViewItem", TreeViewItem, AutoWidgetCell)

void TreeViewItem::setText(const std::wstring& text)
{
	m_text = text;
}

std::wstring TreeViewItem::getText() const
{
	return m_text;
}

void TreeViewItem::setBold(bool bold)
{
}

bool TreeViewItem::isBold() const
{
	return false;
}

void TreeViewItem::setTextOutlineColor(const Color4ub& outlineColor)
{
	m_outlineColor = outlineColor;
}

const Color4ub& TreeViewItem::getTextOutlineColor() const
{
	return m_outlineColor;
}

void TreeViewItem::removeAllImages()
{
	m_images.clear();
}

int32_t TreeViewItem::getImageCount() const
{
	return int32_t(m_images.size());
}

void TreeViewItem::setImage(int32_t index, int32_t image, int32_t expandedImage, int32_t overlayImage)
{
	if (index >= 0 && index < getImageCount())
	{
		Image& img = m_images[index];
		img.image = image;
		img.expanded = expandedImage;
		img.overlay = overlayImage;
	}
}

int32_t TreeViewItem::getImage(int32_t index) const
{
	if (index >= 0 && index < getImageCount())
		return m_images[index].image;
	else
		return -1;
}

int32_t TreeViewItem::getExpandedImage(int32_t index) const
{
	if (index >= 0 && index < getImageCount())
		return m_images[index].expanded;
	else
		return -1;
}

int32_t TreeViewItem::getOverlayImage(int32_t index) const
{
	if (index >= 0 && index < getImageCount())
		return m_images[index].overlay;
	else
		return -1;
}

bool TreeViewItem::isExpanded() const
{
	return m_expanded;
}

void TreeViewItem::expand()
{
	if (!m_expanded)
	{
		m_expanded = true;

		TreeViewItemStateChangeEvent stateChangeEvent(m_view, this);
		m_view->raiseEvent(&stateChangeEvent);
	}
}

bool TreeViewItem::isCollapsed() const
{
	return !m_expanded;
}

void TreeViewItem::collapse()
{
	if (m_expanded)
	{
		m_expanded = false;

		TreeViewItemStateChangeEvent stateChangeEvent(m_view, this);
		m_view->raiseEvent(&stateChangeEvent);
	}
}

bool TreeViewItem::isEnabled() const
{
	return m_enabled;
}

void TreeViewItem::enable()
{
	m_enabled = true;
}

void TreeViewItem::disable()
{
	m_enabled = false;
}

bool TreeViewItem::isSelected() const
{
	return m_selected;
}

void TreeViewItem::select()
{
	m_view->deselectAll();
	m_selected = true;
	m_view->requestUpdate();
}

void TreeViewItem::unselect()
{
	m_selected = false;
}

bool TreeViewItem::isVisible() const
{
	return true;
}

void TreeViewItem::show()
{
	for (TreeViewItem* parent = m_parent; parent; parent = parent->m_parent)
		parent->expand();
	m_view->requestUpdate();
}

void TreeViewItem::setEditable(bool editable)
{
	m_editable = editable;
}

bool TreeViewItem::isEditable() const
{
	return m_editable;
}

bool TreeViewItem::edit()
{
	if (m_editable)
	{
		m_editMode = 0;
		m_view->beginEdit(this);
		return true;
	}
	else
		return false;
}

void TreeViewItem::sort(bool recursive)
{
	if (recursive)
	{
		for (RefArray< TreeViewItem >::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
			(*i)->sort(true);
	}
	m_children.sort(ItemSortPredicate());
}

TreeViewItem* TreeViewItem::getParent() const
{
	return m_parent;
}

TreeViewItem* TreeViewItem::getPreviousSibling(TreeViewItem* child) const
{
	T_FATAL_ASSERT (child->m_parent == this);

	RefArray< TreeViewItem >::const_iterator i = std::find(m_children.begin(), m_children.end(), child);
	if (i == m_children.end() || i == m_children.begin())
		return 0;

	return *(i - 1);
}

TreeViewItem* TreeViewItem::getNextSibling(TreeViewItem* child) const
{
	T_FATAL_ASSERT (child->m_parent == this);

	RefArray< TreeViewItem >::const_iterator i = std::find(m_children.begin(), m_children.end(), child);
	if (i == m_children.end())
		return 0;

	return *(i + 1);
}

bool TreeViewItem::hasChildren() const
{
	return !m_children.empty();
}

const RefArray< TreeViewItem >& TreeViewItem::getChildren() const
{
	return m_children;
}

Ref< TreeViewItem > TreeViewItem::findChild(const std::wstring& childPath)
{
	std::vector< std::wstring > childNames;
	if (Split< std::wstring >::any(childPath, L"/", childNames) <= 0)
		return 0;

	Ref< TreeViewItem > item = this;
	for (std::vector< std::wstring >::iterator i = childNames.begin(); i != childNames.end(); ++i)
	{
		const RefArray< TreeViewItem >& children = item->m_children;

		Ref< TreeViewItem > next;
		for (RefArray< TreeViewItem >::const_iterator j = children.begin(); j != children.end(); ++j)
		{
			if ((*j)->getText() == *i)
			{
				next = *j;
				break;
			}
		}

		if (!(item = next))
			return 0;
	}

	return item;
}

std::wstring TreeViewItem::getPath() const
{
	if (m_parent)
	{
		int32_t count = 0;
		for (RefArray< TreeViewItem >::const_iterator i = m_parent->m_children.begin(); i != m_parent->m_children.end(); ++i)
		{
			if (*i == this)
				break;
			if ((*i)->getText() == m_text)
				++count;
		}
		if (count > 0)
			return m_parent->getPath() + L'/' + m_text + L'[' + toString(count) + L']';
		else
			return m_parent->getPath() + L'/' + m_text;
	}
	else
		return m_text;
}

TreeViewItem::TreeViewItem(TreeView* view, TreeViewItem* parent, const std::wstring& text)
:	m_view(view)
,	m_parent(parent)
,	m_text(text)
,	m_outlineColor(0, 0, 0, 0)
,	m_expanded(false)
,	m_enabled(true)
,	m_selected(false)
,	m_editable(true)
,	m_editMode(0)
,	m_dragMode(0)
{
}

int32_t TreeViewItem::calculateDepth() const
{
	int32_t depth = 0;
	for (const TreeViewItem* item = m_parent; item; item = item->m_parent)
		++depth;
	return depth;
}

Rect TreeViewItem::calculateExpandRect() const
{
	int32_t d = m_view->m_imageState->getSize().cy;
	int32_t depth = calculateDepth();

	Rect rcItem = m_view->getCellClientRect(this);
	rcItem.left += dpi96(4 + depth * 20);
	rcItem.right = rcItem.left + d;

	int32_t dy = (rcItem.getHeight() - d) / 2;
	rcItem.top += dy;
	rcItem.bottom = rcItem.top + d;

	return rcItem;
}

Rect TreeViewItem::calculateImageRect() const
{
	const int32_t d = m_view->m_image->getSize().cy;
	const int32_t depth = calculateDepth();

	Rect rcItem = m_view->getCellClientRect(this);
	rcItem.left += dpi96(4 + depth * 20 + 22);
	rcItem.right = rcItem.left + d;

	int32_t dy = (rcItem.getHeight() - d) / 2;
	rcItem.top += dy;
	rcItem.bottom = rcItem.top + d;

	return rcItem;
}

Rect TreeViewItem::calculateLabelRect() const
{
	const int32_t d = m_view->m_imageState->getSize().cy;
	const int32_t depth = calculateDepth();
	const int32_t imageCount = getImageCount();

	Size extent = m_view->getTextExtent(m_text);

	Rect rcItem = m_view->getCellClientRect(this);
	rcItem.left += dpi96(4 + depth * 20 + 28) + imageCount * d;
	rcItem.right = rcItem.left + extent.cx + d;

	return rcItem;
}

int32_t TreeViewItem::calculateWidth() const
{
	Size extent = m_view->getTextExtent(m_text);
	int32_t d = m_view->m_imageState->getSize().cy;
	return dpi96(4 + calculateDepth() * 20 + 28) + extent.cx + d;
}

void TreeViewItem::interval()
{
	// Cancel pending edit.
	if (m_editMode != 0)
		m_editMode = 0;
}

void TreeViewItem::mouseDown(MouseButtonDownEvent* event, const Point& position)
{
	m_mouseDownPosition = position;
	m_dragMode = 0;

	if (hasChildren() && calculateExpandRect().inside(event->getPosition()))
	{
		if (m_expanded)
			collapse();
		else
			expand();
	}
	else
	{
		if (!isSelected())
		{
			// Select this item only.
			m_view->deselectAll();
			m_selected = true;

			SelectionChangeEvent selectionChangeEvent(m_view);
			m_view->raiseEvent(&selectionChangeEvent);
		}

		if (calculateLabelRect().inside(event->getPosition()))
		{
			if (m_editable)
			{
				if (m_editMode == 0)
				{
					// Wait for next tap; cancel wait after 2 seconds.
					m_view->requestInterval(this, 2000);
					m_editMode = 1;
				}
				else if (m_editMode == 1)
				{
					// Double tap detected; begin edit after mouse is released.
					m_view->requestInterval(this, 1000);
					m_editMode = 2;
				}
			}
			m_dragMode = 1;
		}
	}

	m_view->requestUpdate();
}

void TreeViewItem::mouseUp(MouseButtonUpEvent* event, const Point& position)
{
	if (m_editMode == 2)
	{
		T_ASSERT (m_editable);
		if (m_view->m_autoEdit)
			m_view->beginEdit(this);
		m_editMode = 0;
	}
	if (m_dragMode == 2)
	{
		if (!m_view->getInnerRect().inside(event->getPosition()))
		{
			Point position = m_view->clientToScreen(event->getPosition());
			TreeViewDragEvent dragEvent(m_view, this, TreeViewDragEvent::DmDrop, position);
			m_view->raiseEvent(&dragEvent);
		}
	}
	m_dragMode = 0;
}

void TreeViewItem::mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position)
{
	// Ensure edit isn't triggered.
	m_editMode = 0;

	// Raise activation event.
	TreeViewItemActivateEvent activateEvent(m_view, this);
	m_view->raiseEvent(&activateEvent);
}

void TreeViewItem::mouseMove(MouseMoveEvent* event, const Point& position)
{
	Size d = position - m_mouseDownPosition;
	if (abs(d.cx) > dpi96(2) || abs(d.cy) > dpi96(2))
	{
		// Ensure edit isn't triggered if mouse moved during edit state tracking.
		m_editMode = 0;

		if (m_dragMode == 1)
		{
			TreeViewDragEvent dragEvent(m_view, this, TreeViewDragEvent::DmDrag);
			m_view->raiseEvent(&dragEvent);
			if (!(dragEvent.consumed() && dragEvent.cancelled()))
				m_dragMode = 2;
			else
				m_dragMode = 3;
		}
	}
}

void TreeViewItem::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	if (isSelected())
	{
		canvas.setBackground(ss->getColor(m_view, L"item-background-color-selected"));
		canvas.fillRect(rect);
	}

	if (m_view->m_imageState && hasChildren())
	{
		Rect rcExpand = calculateExpandRect();
		int32_t d = m_view->m_imageState->getSize().cy;
		canvas.drawBitmap(
			rcExpand.getTopLeft(),
			Point(isExpanded() ? d : 0, 0),
			Size(d, d),
			m_view->m_imageState,
			BmAlpha
		);
	}

	if (m_view->m_image)
	{
		const int32_t d = m_view->m_image->getSize().cy;

		for (int32_t i = 0; i < getImageCount(); ++i)
		{
			int32_t image = (hasChildren() && isExpanded()) ? getExpandedImage(i) : getImage(i);
			if (image < 0)
				image = getImage(i);

			Rect rcImage = calculateImageRect();

			if (image >= 0)
				canvas.drawBitmap(
					rcImage.getTopLeft() + Size(i * d, 0),
					Point(image * d, 0),
					Size(d, d),
					m_view->m_image,
					BmAlpha
				);

			if (getOverlayImage(i) >= 0)
				canvas.drawBitmap(
					rcImage.getTopLeft() + Size(i * d, 0),
					Point(getOverlayImage(i) * d, 0),
					Size(d, d),
					m_view->m_image,
					BmAlpha
				);
		}
	}

	if (!m_text.empty())
	{
		Rect rcLabel = calculateLabelRect();

		if (m_outlineColor.a != 0)
		{
			canvas.setForeground(m_outlineColor);
			for (int32_t dy = -1; dy <= 1; ++dy)
			{
				for (int32_t dx = -1; dx <= 1; ++dx)
				{
					if (dx == 0 && dy == 0)
						continue;
					canvas.drawText(rcLabel.offset(dx, dy), m_text, AnLeft, AnCenter);
				}
			}
		}

		if (m_enabled)
			canvas.setForeground(ss->getColor(m_view, m_selected ? L"item-color-selected" : L"color"));
		else
			canvas.setForeground(ss->getColor(m_view, m_selected ? L"item-color-selected-disabled" : L"color-disabled"));

		canvas.drawText(rcLabel, m_text, AnLeft, AnCenter);
	}
}

		}
	}
}
