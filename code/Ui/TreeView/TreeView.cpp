/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <stack>
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Ui/Edit.h"
#include "Ui/HierarchicalState.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewContentChangeEvent.h"
#include "Ui/TreeView/TreeViewEditEvent.h"
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/TreeView/TreeViewItemActivateEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeView", TreeView, AutoWidget)

bool TreeView::create(Widget* parent, uint32_t style)
{
	if (!AutoWidget::create(parent, style | WsFocus | WsWantAllInput))
		return false;

	m_autoEdit = bool((style & WsAutoEdit) == WsAutoEdit);

	m_itemEditor = new Edit();
	m_itemEditor->create(this, L"", WsBorder | WsWantAllInput);
	m_itemEditor->hide();
	m_itemEditor->addEventHandler< FocusEvent >(this, &TreeView::eventEditFocus);
	m_itemEditor->addEventHandler< KeyDownEvent >(this, &TreeView::eventEditKeyDownEvent);

	m_imageState = new ui::StyleBitmap(L"UI.Tree");

	m_font = getFont();

	m_fontBold = m_font;
	m_fontBold.setBold(true);

	addEventHandler< ScrollEvent >(this, &TreeView::eventScroll);
	addEventHandler< KeyDownEvent >(this, &TreeView::eventKeyDown);
	return true;
}

int32_t TreeView::addImage(IBitmap* image)
{
	const int32_t base = (int32_t)m_images.size();
	m_images.push_back(image);
	return base;
}

bool TreeView::setImage(int32_t imageIndex, IBitmap* image)
{
	if (imageIndex < 0 || imageIndex >= (int32_t)m_images.size())
		return false;

	m_images[imageIndex] = image;
	return true;
}

void TreeView::removeAllImages()
{
	m_images.clear();
}

Ref< TreeViewItem > TreeView::createItem(TreeViewItem* parent, const std::wstring& text, int32_t imageColumns)
{
	Ref< TreeViewItem > item = new TreeViewItem(this, parent, text);

	if (parent)
		parent->m_children.push_back(item);
	else
		m_roots.push_back(item);

	if (imageColumns > 0)
		item->m_images.resize(imageColumns);

	requestUpdate();
	return item;
}

void TreeView::removeItem(TreeViewItem* item)
{
	if (item->m_parent)
	{
		T_ANONYMOUS_VAR(Ref< TreeViewItem >)(item);
		item->m_parent->m_children.remove(item);
	}
	else
		m_roots.remove(item);

	requestUpdate();
}

void TreeView::removeAllItems()
{
	m_roots.resize(0);
	requestUpdate();
}

uint32_t TreeView::getItems(RefArray< TreeViewItem >& outItems, uint32_t flags) const
{
	typedef std::pair< RefArray< TreeViewItem >::const_iterator, RefArray< TreeViewItem >::const_iterator > range_t;

	std::stack< range_t > stack;
	stack.push(std::make_pair(m_roots.begin(), m_roots.end()));

	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			TreeViewItem* item = *r.first++;

			if (flags & GfSelectedOnly)
			{
				if (item->isSelected())
					outItems.push_back(item);
			}
			else
				outItems.push_back(item);

			if (flags & GfDescendants)
			{
				if ((flags & GfExpandedOnly) != GfExpandedOnly || item->isExpanded())
				{
					const RefArray< TreeViewItem >& children = item->getChildren();
					if (!children.empty())
						stack.push(std::make_pair(children.begin(), children.end()));
				}
			}
		}
		else
			stack.pop();
	}

	return uint32_t(outItems.size());
}

void TreeView::deselectAll()
{
	RefArray< TreeViewItem > selectedItems;
	getItems(selectedItems, TreeView::GfDescendants | TreeView::GfSelectedOnly);
	for (auto selectedItem : selectedItems)
		selectedItem->unselect();
}

Ref< HierarchicalState > TreeView::captureState() const
{
	Ref< HierarchicalState > state = new HierarchicalState();
	RefArray< TreeViewItem > items;
	getItems(items, GfDescendants);
	for (auto item : items)
	{
		state->addState(
			item->getPath(),
			item->isExpanded(),
			item->isSelected()
		);
	}
	return state;
}

void TreeView::applyState(const HierarchicalState* state)
{
	RefArray< TreeViewItem > items;
	getItems(items, GfDescendants);
	for (auto item : items)
	{
		const std::wstring path = item->getPath();

		if (state->getExpanded(path))
			item->expand();
		else
			item->collapse();

		if (state->getSelected(path))
			item->select();
		else
			item->unselect();
	}
}

int32_t TreeView::getMaxImageHeight() const
{
	int32_t maxImageHeight = 0;
	for (auto image : m_images)
		maxImageHeight = std::max(maxImageHeight, image->getSize(this).cy);
	return maxImageHeight;
}

void TreeView::layoutCells(const Rect& rc)
{
	int32_t height = getFontMetric().getHeight() + pixel(6_ut);

	RefArray< TreeViewItem > items;
	getItems(items, GfDescendants | GfExpandedOnly);

	int32_t maxWidth = rc.right - rc.left;
	for (auto item : items)
		maxWidth = std::max(maxWidth, rc.left + item->calculateWidth());

	Rect rcRow(rc.left, rc.top, rc.left + maxWidth, rc.top + height);
	for (auto item : items)
	{
		placeCell(item, rcRow);
		rcRow = rcRow.offset(0, height);
	}

	m_itemEditor->hide();
}

void TreeView::beginEdit(TreeViewItem* item)
{
	TreeViewEditEvent editEvent(this, item);
	raiseEvent(&editEvent);
	if (editEvent.consumed() && editEvent.cancelled())
		return;

	releaseCapturedCell();

	Rect rcLabel = item->calculateLabelRect();
	Rect rcItem = item->getClientRect();

	m_itemEditor->setRect(Rect(rcLabel.left, rcItem.top, rcItem.right, rcItem.bottom));
	m_itemEditor->setText(item->getText());
	m_itemEditor->selectAll();
	m_itemEditor->show();
	m_itemEditor->setFocus();

	m_editItem = item;
}

void TreeView::eventEditFocus(FocusEvent* event)
{
	if (event->lostFocus() && m_itemEditor->isVisible(false))
	{
		std::wstring originalText = m_editItem->getText();
		std::wstring newText = m_itemEditor->getText();

		m_itemEditor->hide();

		m_editItem->setText(newText);

		TreeViewContentChangeEvent changeEvent(this, m_editItem, originalText);
		raiseEvent(&changeEvent);

		if (!changeEvent.consumed())
			m_editItem->setText(originalText);

		event->consume();
	}
}

void TreeView::eventEditKeyDownEvent(KeyDownEvent* event)
{
	if (event->getVirtualKey() == ui::VkReturn)
	{
		std::wstring originalText = m_editItem->getText();
		std::wstring newText = m_itemEditor->getText();

		m_itemEditor->hide();

		m_editItem->setText(newText);

		TreeViewContentChangeEvent changeEvent(this, m_editItem, originalText);
		raiseEvent(&changeEvent);

		if (!changeEvent.consumed())
			m_editItem->setText(originalText);
	}
	else if (event->getVirtualKey() == ui::VkEscape)
	{
		m_itemEditor->hide();
	}
}

void TreeView::eventScroll(ScrollEvent* event)
{
	m_itemEditor->hide();
}

void TreeView::eventKeyDown(KeyDownEvent* event)
{
	RefArray< TreeViewItem > items;
	getItems(items, TreeView::GfDescendants | TreeView::GfExpandedOnly);

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

	const bool recursive = (bool)((event->getKeyState() & KsShift) != 0);

	switch (event->getVirtualKey())
	{
	case VkLeft:
		if (items[current]->isExpanded())
			items[current]->collapse(recursive);
		else if (items[current]->getParent() != 0)
			items[current]->getParent()->select();
		break;

	case VkRight:
		if (items[current]->hasChildren())
		{
			if (items[current]->isCollapsed())
				items[current]->expand(recursive);
			else
				items[current + 1]->select();
		}
		break;

	case VkUp:
		if (current > 0)
			items[current - 1]->select();
		break;

	case VkDown:
		if (current < items.size() - 1)
			items[current + 1]->select();
		break;

	case VkReturn:
		{
			TreeViewItemActivateEvent activateEvent(this, items[current]);
			raiseEvent(&activateEvent);
		}
		break;

	default:
		break;
	}

	requestUpdate();
}

}
