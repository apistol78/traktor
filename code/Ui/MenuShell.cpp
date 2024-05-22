/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MenuShell.h"
#include "Ui/ScrollBar.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MenuShell", MenuShell, Widget)

bool MenuShell::create(Widget* parent, int32_t maxItems)
{
	if (!Widget::create(parent, ui::WsDoubleBuffer))
		return false;

	addEventHandler< MouseMoveEvent >(this, &MenuShell::eventMouseMove);
	addEventHandler< PaintEvent >(this, &MenuShell::eventPaint);
	addEventHandler< SizeEvent >(this, &MenuShell::eventSize);

	if ((m_maxItems = maxItems) > 0)
	{
		m_scrollBar = new ScrollBar();
		if (!m_scrollBar->create(this, ScrollBar::WsVertical))
			return false;

		m_scrollBar->addEventHandler< ScrollEvent >(this, &MenuShell::eventScroll);
	}

	m_eventHandlerButtonDown = Application::getInstance()->addEventHandler< MouseButtonDownEvent >(this, &MenuShell::eventGlobalButtonDown);
	m_eventHandlerButtonUp = Application::getInstance()->addEventHandler< MouseButtonUpEvent >(this, &MenuShell::eventGlobalButtonUp);

	return true;
}

void MenuShell::destroy()
{
	Application::getInstance()->removeEventHandler(m_eventHandlerButtonUp);
	Application::getInstance()->removeEventHandler(m_eventHandlerButtonDown);

	Widget::destroy();
}

void MenuShell::add(MenuItem* item)
{
	m_items.push_back(item);

	if (m_scrollBar)
	{
		m_scrollBar->setRange((int32_t)m_items.size());
		m_scrollBar->setPage(m_maxItems - 1);
		m_scrollBar->setVisible(m_items.size() > m_maxItems);
	}
}

MenuItem* MenuShell::getItem(const Point& at) const
{
	const Rect rcInner = getInnerRect();
	int32_t itemWidth = rcInner.getWidth() - 2;
	Point itemTopLeft(1, 1);

	if (m_scrollBar != nullptr && m_scrollBar->isVisible(false))
	{
		itemWidth -= m_scrollBar->getPreferredSize(rcInner.getSize()).cx;
		itemTopLeft.y = 1 - m_scrollBar->getPosition() * m_items.front()->getSize(this).cy;
	}

	for (auto item : m_items)
	{
		const Size itemSize(itemWidth, item->getSize(this).cy);
		if (Rect(itemTopLeft, itemSize).inside(at))
			return item;
		itemTopLeft.y += itemSize.cy;
	}

	return nullptr;
}

bool MenuShell::getItemRect(const MenuItem* item, Rect& outItemRect) const
{
	const Rect rcInner = getInnerRect();
	int32_t itemWidth = rcInner.getWidth() - 2;
	Point itemTopLeft(1, 1);

	if (m_scrollBar != nullptr && m_scrollBar->isVisible(false))
	{
		itemWidth -= m_scrollBar->getPreferredSize(rcInner.getSize()).cx;
		itemTopLeft.y = 1 - m_scrollBar->getPosition() * m_items.front()->getSize(this).cy;
	}

	for (auto it : m_items)
	{
		const Size itemSize(itemWidth, item->getSize(this).cy);
		if (it == item)
		{
			outItemRect = Rect(itemTopLeft, itemSize);
			return true;
		}
		itemTopLeft.y += itemSize.cy;
	}

	return false;
}

Size MenuShell::getMinimumSize() const
{
	const Rect rcInner = getInnerRect();

	Size minimumSize(0, 0);
	for (auto item : m_items)
	{
		const Size itemSize = item->getSize(this);
		minimumSize.cx = std::max(minimumSize.cx, itemSize.cx);
		minimumSize.cy += itemSize.cy;
	}

	// Limit maximum number of visible items; assume all items has same height.
	if (m_maxItems > 0)
	{
		const int32_t numItems = std::min(m_maxItems, (int32_t)m_items.size());
		minimumSize.cy = m_items.front()->getSize(this).cy * numItems;
		if (m_scrollBar->isVisible(false))
			minimumSize.cx += m_scrollBar->getPreferredSize(rcInner.getSize()).cx;
	}

	return minimumSize + Size(2, 2);
}

Size MenuShell::getPreferredSize(const Size& hint) const
{
	return getMinimumSize();
}

void MenuShell::eventMouseMove(MouseMoveEvent* event)
{
	MenuItem* item = getItem(event->getPosition());
	if (item != m_trackItem)
	{
		safeDestroy(m_trackSubMenu);

		if ((m_trackItem = item) != nullptr)
		{
			if (item->count() > 0)
			{
				Menu sm;

				// Build sub menu.
				for (int32_t i = 0; i < item->count(); ++i)
					sm.add(item->get(i));

				// Get position of clicked item.
				Rect rc;
				getItemRect(item, rc);

				// Show sub menu.
				m_trackSubMenu = sm.show(this, Point(rc.right, rc.top));
				T_FATAL_ASSERT (m_trackSubMenu != nullptr);

				m_trackSubMenu->addEventHandler< MenuClickEvent >([&](MenuClickEvent* e) {
					// Sub item selected, re-issue event in this shell.
					MenuClickEvent clickEvent(this, e->getItem(), e->getCommand());
					raiseEvent(&clickEvent);
				});
			}
		}

		update();
	}
}

void MenuShell::eventGlobalButtonDown(MouseButtonDownEvent* event)
{
	m_activeItem = nullptr;

	// If sub menu already created then lets assume it's event handler perform click event.
	if (m_trackSubMenu)
		return;

	// Do not cancel if clicking on scrollbar.
	if (
		m_scrollBar != nullptr &&
		m_scrollBar->isVisible(false) &&
		m_scrollBar->hitTest(event->getPosition())
	)
		return;

	// Get active item from mouse position.
	const Point clientPosition = screenToClient(event->getPosition());
	MenuItem* item = getItem(clientPosition);
	if (item != nullptr)
	{
		if (item->isEnable())
			m_activeItem = item;
	}
	else
	{
		MenuClickEvent clickEvent(this, nullptr, Command());
		raiseEvent(&clickEvent);
	}
}

void MenuShell::eventGlobalButtonUp(MouseButtonUpEvent* event)
{
	// If sub menu already created then lets assume it's event handler perform click event.
	if (!m_activeItem || m_trackSubMenu)
		return;

	// Do not cancel if clicking on scrollbar.
	if (
		m_scrollBar != nullptr &&
		m_scrollBar->isVisible(false) &&
		m_scrollBar->hitTest(event->getPosition())
	)
		return;

	const Point clientPosition = screenToClient(event->getPosition());
	MenuItem* item = getItem(clientPosition);
	if (item == m_activeItem)
	{
		// Toggle if checkbox item selected.
		if (item->getCheckBox())
			item->setChecked(!item->isChecked());

		MenuClickEvent clickEvent(this, item, item->getCommand());
		raiseEvent(&clickEvent);
	}
}

void MenuShell::eventPaint(PaintEvent* e)
{
	Canvas& canvas = e->getCanvas();
	const Rect rcInner = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	int32_t itemWidth = rcInner.getWidth() - 2;
	Point itemTopLeft(1, 1);

	if (m_scrollBar != nullptr && m_scrollBar->isVisible(false))
	{
		itemWidth -= m_scrollBar->getPreferredSize(rcInner.getSize()).cx;
		itemTopLeft.y = 1 - m_scrollBar->getPosition() * m_items.front()->getSize(this).cy;
	}

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	for (auto item : m_items)
	{
		const Size itemSize(itemWidth, item->getSize(this).cy);
		item->paint(this, canvas, Rect(itemTopLeft, itemSize), bool(item == m_trackItem));
		itemTopLeft.y += itemSize.cy;
	}

	canvas.setForeground(ss->getColor(this, L"border-color"));
	canvas.drawRect(rcInner);

	e->consume();
}

void MenuShell::eventSize(SizeEvent* e)
{
	if (m_scrollBar != nullptr)
	{
		const Rect rcInner = getInnerRect();
		const Size szPreferred = m_scrollBar->getPreferredSize(rcInner.getSize());

		m_scrollBar->setRect(Rect(
			rcInner.right - szPreferred.cx - 1, rcInner.top + 1,
			rcInner.right - 1, rcInner.bottom - 1
		));
	}
}

void MenuShell::eventScroll(ScrollEvent* e)
{
	update(nullptr, false);
}

}
