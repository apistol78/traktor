/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/PreviewList/PreviewContentChangeEvent.h"
#include "Ui/PreviewList/PreviewItem.h"
#include "Ui/PreviewList/PreviewItems.h"
#include "Ui/PreviewList/PreviewList.h"
#include "Ui/PreviewList/PreviewSelectionChangeEvent.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_marginX = 10_ut;
const Unit c_marginY = 10_ut;
const Unit c_itemWidth = 120_ut;
const Unit c_itemHeight = 110_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PreviewList", PreviewList, AutoWidget)

bool PreviewList::create(Widget* parent, uint32_t style)
{
	if (!AutoWidget::create(parent, style | WsFocus))
		return false;

	m_single = bool((style & WsMultiple) == 0);

	m_itemEditor = new Edit();
	m_itemEditor->create(this, L"", WsBorder | WsWantAllInput);
	m_itemEditor->hide();
	m_itemEditor->addEventHandler< FocusEvent >(this, &PreviewList::eventEditFocus);
	m_itemEditor->addEventHandler< KeyDownEvent >(this, &PreviewList::eventEditKeyDownEvent);

	addEventHandler< MouseButtonDownEvent >(this, &PreviewList::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &PreviewList::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &PreviewList::eventMouseMove);
	return true;
}

void PreviewList::setItems(PreviewItems* items)
{
	m_items = items;
	requestUpdate();
}

Ref< PreviewItems > PreviewList::getItems() const
{
	return m_items;
}

PreviewItem* PreviewList::getSelectedItem() const
{
	if (!m_items)
		return nullptr;

	for (int32_t i = 0; i < m_items->count(); ++i)
	{
		if (m_items->get(i)->isSelected())
			return m_items->get(i);
	}

	return nullptr;
}

void PreviewList::getSelectedItems(RefArray< PreviewItem >& outItems) const
{
	if (!m_items)
		return;

	for (int32_t i = 0; i < m_items->count(); ++i)
	{
		if (m_items->get(i)->isSelected())
			outItems.push_back(m_items->get(i));
	}
}

void PreviewList::beginEdit(PreviewItem* item)
{
	const Rect rcItem = item->getTextRect();

	m_itemEditor->setRect(rcItem);
	m_itemEditor->setText(item->getText());
	m_itemEditor->selectAll();
	m_itemEditor->show();
	m_itemEditor->setFocus();

	m_editItem = item;
}

void PreviewList::layoutCells(const Rect& rc)
{
	const int32_t nitems = m_items ? m_items->count() : 0;
	if (nitems <= 0)
		return;

	const int32_t width = pixel(c_itemWidth);
	const int32_t height = pixel(c_itemHeight);

	const int32_t ncolumns = (rc.getWidth() - pixel(c_marginX * 2_ut)) / width;
	if (ncolumns <= 0)
		return;

	const int32_t pad = pixel(4_ut);

	// First layout non selected.
	for (int32_t i = 0; i < nitems; ++i)
	{
		if (m_items->get(i)->isSelected())
			continue;

		const int32_t column = i % ncolumns;
		const int32_t row = i / ncolumns;

		const Rect rcItem(
			pixel(c_marginX) + column * width,
			pixel(c_marginY) + row * height,
			pixel(c_marginX) + column * width + width,
			pixel(c_marginY) + row * height + height
		);

		placeCell(m_items->get(i), rcItem.inflate(-pad, -pad));
	}

	// Then layout selected items.
	for (int32_t i = 0; i < nitems; ++i)
	{
		if (!m_items->get(i)->isSelected())
			continue;

		const int32_t column = i % ncolumns;
		const int32_t row = i / ncolumns;

		const Rect rcItem(
			pixel(c_marginX) + column * width,
			pixel(c_marginY) + row * height,
			pixel(c_marginX) + column * width + width,
			pixel(c_marginY) + row * height + height
		);

		placeCell(m_items->get(i), rcItem.inflate(-pad, -pad));
	}
}

void PreviewList::eventEditFocus(FocusEvent* event)
{
	if (event->lostFocus() && m_itemEditor->isVisible(false))
	{
		const std::wstring originalText = m_editItem->getText();
		const std::wstring newText = m_itemEditor->getText();

		m_itemEditor->hide();

		m_editItem->setText(newText);

		PreviewContentChangeEvent changeEvent(this, m_editItem, originalText);
		raiseEvent(&changeEvent);

		if (!changeEvent.consumed())
			m_editItem->setText(originalText);

		event->consume();
	}
}


void PreviewList::eventEditKeyDownEvent(KeyDownEvent* event)
{
	if (event->getVirtualKey() == ui::VkReturn)
	{
		const std::wstring originalText = m_editItem->getText();
		const std::wstring newText = m_itemEditor->getText();

		m_itemEditor->hide();

		m_editItem->setText(newText);

		PreviewContentChangeEvent changeEvent(this, m_editItem, originalText);
		raiseEvent(&changeEvent);

		if (!changeEvent.consumed())
			m_editItem->setText(originalText);
	}
	else if (event->getVirtualKey() == ui::VkEscape)
	{
		m_itemEditor->hide();
	}
}

void PreviewList::eventButtonDown(MouseButtonDownEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	m_dragMode = 0;
	m_dragOriginPosition = event->getPosition();

	const Point& position = event->getPosition();
	if (m_items)
	{
		// De-select all items first.
		if (m_single || (event->getKeyState() & (KsShift | KsControl)) == 0)
		{
			for (int32_t i = 0; i < m_items->count(); ++i)
				m_items->get(i)->setSelected(false);
		}

		// Get item from position.
		AutoWidgetCell* cell = hitTest(position);
		if (PreviewItem* item = dynamic_type_cast< PreviewItem* >(cell))
		{
			item->setSelected(true);

			PreviewSelectionChangeEvent selectionChangeEvent(this, item);
			raiseEvent(&selectionChangeEvent);

			m_dragMode = 1;
		}
		else
		{
			PreviewSelectionChangeEvent selectionChangeEvent(this, nullptr);
			raiseEvent(&selectionChangeEvent);
		}
	}

	requestUpdate();
}

void PreviewList::eventButtonUp(MouseButtonUpEvent* event)
{
	if (m_dragMode == 2)
	{
		const Point position = clientToScreen(event->getPosition());
		DragEvent dragEvent(this, DragEvent::Moment::Drop, position);
		raiseEvent(&dragEvent);
	}
	m_dragMode = 0;
}

void PreviewList::eventMouseMove(MouseMoveEvent* event)
{
	const Size d = event->getPosition() - m_dragOriginPosition;
	if (abs(d.cx) > pixel(2_ut) || abs(d.cy) > pixel(2_ut))
	{
		if (m_dragMode == 1)
		{
			DragEvent dragEvent(this, DragEvent::Moment::Drag);
			raiseEvent(&dragEvent);

			if (!(dragEvent.consumed() && dragEvent.cancelled()))
				m_dragMode = 2;
			else
				m_dragMode = 0;
		}
	}
}

}
