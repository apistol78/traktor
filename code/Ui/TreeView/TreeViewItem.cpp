/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/IBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewDragEvent.h"
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/TreeView/TreeViewItemActivateEvent.h"
#include "Ui/TreeView/TreeViewItemStateChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewItem", TreeViewItem, AutoWidgetCell)

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
	m_bold = bold;
}

bool TreeViewItem::isBold() const
{
	return m_bold;
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

void TreeViewItem::expand(bool recursive)
{
	if (!m_expanded)
	{
		m_expanded = true;

		TreeViewItemStateChangeEvent stateChangeEvent(m_view, this);
		m_view->raiseEvent(&stateChangeEvent);
	}
	if (recursive)
	{
		for (auto child : m_children)
			child->expand(true);
	}
}

bool TreeViewItem::isCollapsed() const
{
	return !m_expanded;
}

void TreeViewItem::collapse(bool recursive)
{
	if (m_expanded)
	{
		m_expanded = false;

		TreeViewItemStateChangeEvent stateChangeEvent(m_view, this);
		m_view->raiseEvent(&stateChangeEvent);
	}
	if (recursive)
	{
		for (auto child : m_children)
			child->collapse(true);
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
	// Expand all parents to ensure this item is visible.
	for (TreeViewItem* parent = m_parent; parent; parent = parent->m_parent)
		parent->expand();

	// Layout all cells immediately.
	m_view->updateLayout();

	// Scroll view to this item.
	Rect rc = getRect();
	m_view->scrollTo({ 0, rc.getCenter().y });
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
	sort(recursive, [](const TreeViewItem* item1, const TreeViewItem* item2) -> bool {
		return compareIgnoreCase(item1->getText(), item2->getText()) < 0;
	});
}

void TreeViewItem::sort(bool recursive, const std::function< bool(const TreeViewItem* item1, const TreeViewItem* item2) >& predicate)
{
	if (recursive)
	{
		for (auto child : m_children)
			child->sort(true, predicate);
	}
	m_children.sort(predicate);
}

TreeViewItem* TreeViewItem::getParent() const
{
	return m_parent;
}

TreeViewItem* TreeViewItem::getPreviousSibling(TreeViewItem* child) const
{
	T_FATAL_ASSERT (child->m_parent == this);

	auto i = std::find(m_children.begin(), m_children.end(), child);
	if (i == m_children.end() || i == m_children.begin())
		return nullptr;

	return *(i - 1);
}

TreeViewItem* TreeViewItem::getNextSibling(TreeViewItem* child) const
{
	T_FATAL_ASSERT (child->m_parent == this);

	auto i = std::find(m_children.begin(), m_children.end(), child);
	if (i == m_children.end())
		return nullptr;

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
		return nullptr;

	Ref< TreeViewItem > item = this;
	for (const auto& childName : childNames)
	{
		const RefArray< TreeViewItem >& children = item->m_children;

		Ref< TreeViewItem > next;
		for (auto child : children)
		{
			if (child->getText() == childName)
			{
				next = child;
				break;
			}
		}

		if (!(item = next))
			return nullptr;
	}

	return item;
}

std::wstring TreeViewItem::getPath() const
{
	if (m_parent)
	{
		int32_t count = 0;
		for (auto child : m_parent->m_children)
		{
			if (child == this)
				break;
			if (child->getText() == m_text)
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
,	m_bold(false)
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
	int32_t d = m_view->m_imageState->getSize(getWidget()).cy;
	int32_t depth = calculateDepth();

	Rect rcItem = getClientRect();
	rcItem.left += m_view->pixel(4_ut + Unit(depth) * 20_ut);
	rcItem.right = rcItem.left + d;

	int32_t dy = (rcItem.getHeight() - d) / 2;
	rcItem.top += dy;
	rcItem.bottom = rcItem.top + d;

	return rcItem;
}

Rect TreeViewItem::calculateImageRect() const
{
	const int32_t d = m_view->getMaxImageHeight();
	const int32_t depth = calculateDepth();
	const int32_t imageCount = getImageCount();

	Rect rcItem = getClientRect();
	rcItem.left += m_view->pixel(4_ut + Unit(depth) * 20_ut) + d;
	rcItem.right = rcItem.left + imageCount * d;

	const int32_t dy = (rcItem.getHeight() - d) / 2;
	rcItem.top += dy;
	rcItem.bottom = rcItem.top + d;

	return rcItem;
}

Rect TreeViewItem::calculateLabelRect() const
{
	const int32_t d = m_view->m_imageState->getSize(getWidget()).cy;
	const int32_t depth = calculateDepth();
	const int32_t imageCount = getImageCount();

	const Size extent = m_view->getFontMetric().getExtent(m_text);

	Rect rcItem = getClientRect();
	rcItem.left += m_view->pixel(4_ut + Unit(depth) * 20_ut) + d + imageCount * d + (imageCount > 0 ? m_view->pixel(4_ut) : 0);
	rcItem.right = rcItem.left + extent.cx + d;

	return rcItem;
}

int32_t TreeViewItem::calculateWidth() const
{
	const Size extent = m_view->getFontMetric().getExtent(m_text);
	const int32_t d = m_view->m_imageState->getSize(getWidget()).cy;
	return m_view->pixel(4_ut + Unit(calculateDepth()) * 20_ut + 28_ut) + extent.cx + d;
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

		m_view->requestUpdate();
	}
	else
	{
		if (!isSelected())
		{
			// Select this item only.
			m_view->deselectAll();
			m_selected = true;

			// Update immediately because we possibly want user to notice selection
			// change before popups etc.
			m_view->update(nullptr, true);

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
}

void TreeViewItem::mouseUp(MouseButtonUpEvent* event, const Point& position)
{
	if (m_editMode == 2)
	{
		T_ASSERT(m_editable);
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

	// If event hasn't been consumed and we have children then toggle expanded state.
	if (!activateEvent.consumed())
	{
		if (m_expanded)
			collapse();
		else
			expand();

		m_view->requestUpdate();
	}
}

void TreeViewItem::mouseMove(MouseMoveEvent* event, const Point& position)
{
	Size d = position - m_mouseDownPosition;
	if (abs(d.cx) > m_view->pixel(2_ut) || abs(d.cy) > m_view->pixel(2_ut))
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
	const StyleSheet* ss = m_view->getStyleSheet();

	if (isSelected())
	{
		canvas.setBackground(ss->getColor(this, L"background-color-selected"));
		canvas.fillRect(rect);
	}
	else if (m_view->getHoverCell() == this)
	{
		canvas.setBackground(ss->getColor(this, L"background-color-hover"));
		canvas.fillRect(rect);
	}

	if (m_view->m_imageState && hasChildren())
	{
		Rect rcExpand = calculateExpandRect();
		int32_t d = m_view->m_imageState->getSize(getWidget()).cy;
		canvas.drawBitmap(
			rcExpand.getTopLeft(),
			Point(isExpanded() ? d : 0, 0),
			Size(d, d),
			m_view->m_imageState,
			BlendMode::Alpha
		);
	}

	const int32_t d = m_view->getMaxImageHeight();
	for (int32_t i = 0; i < getImageCount(); ++i)
	{
		int32_t imageIndex = (hasChildren() && isExpanded()) ? getExpandedImage(i) : getImage(i);
		if (imageIndex < 0)
			imageIndex = getImage(i);

		if (imageIndex < 0 || imageIndex >= (int32_t)m_view->m_images.size())
			continue;

		const Rect rcImage = calculateImageRect();
		canvas.drawBitmap(
			rcImage.getTopLeft() + Size(i * d, 0),
			Point(0, 0),
			m_view->m_images[imageIndex]->getSize(m_view),
			m_view->m_images[imageIndex],
			BlendMode::Alpha
		);

		//if (getOverlayImage(i) >= 0)
		//	canvas.drawBitmap(
		//		rcImage.getTopLeft() + Size(i * d, 0),
		//		Point(getOverlayImage(i) * d, 0),
		//		Size(d, d),
		//		m_view->m_image,
		//		BlendMode::Alpha
		//	);
	}

	if (!m_text.empty())
	{
		Rect rcLabel = calculateLabelRect();

		if (m_bold)
			canvas.setFont(m_view->m_fontBold);

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
			canvas.setForeground(ss->getColor(m_view, m_selected ? L"color-selected" : L"color"));
		else
			canvas.setForeground(ss->getColor(m_view, m_selected ? L"color-selected-disabled" : L"color-disabled"));

		canvas.drawText(rcLabel, m_text, AnLeft, AnCenter);

		if (m_bold)
			canvas.setFont(m_view->m_font);
	}
}

	}
}
