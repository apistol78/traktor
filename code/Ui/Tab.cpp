/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/Font.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Tab", Tab, Widget)

Tab::Tab()
:	m_imageWidth(0)
,	m_imageHeight(0)
,	m_closeButton(false)
,	m_drawBorder(false)
,	m_drawLine(false)
,	m_bottom(false)
{
}

bool Tab::create(Widget* parent, int32_t style)
{
	if (!Widget::create(parent, style & ~WsBorder))
		return false;

	addEventHandler< MouseTrackEvent >(this, &Tab::eventMouseTrack);
	addEventHandler< MouseMoveEvent >(this, &Tab::eventMouseMove);
	addEventHandler< MouseButtonDownEvent >(this, &Tab::eventButtonDown);
	addEventHandler< SizeEvent >(this, &Tab::eventSize);
	addEventHandler< PaintEvent >(this, &Tab::eventPaint);

	m_fontBold = getFont();
	m_fontBold.setBold(true);

	m_closeButton = bool((style & WsCloseButton) == WsCloseButton);
	m_drawBorder = bool((style & WsBorder) == WsBorder);
	m_drawLine = bool((style & WsLine) == WsLine);
	m_bottom = bool((style & WsBottom) == WsBottom);

	m_bitmapClose = new StyleBitmap(L"UI.TabClose");
	m_tabHeight = getFont().getSize() + 12_ut;
	return true;
}

Rect Tab::getInnerRect() const
{
	return m_innerRect;
}

int32_t Tab::addImage(IBitmap* image, int32_t imageCount)
{
	uint32_t width = 0, height = 0;

	// Resize existing image.
	if (m_bitmapImages)
	{
		width = m_bitmapImages->getSize(this).cx + image->getSize(this).cx;
		height = std::max(m_bitmapImages->getSize(this).cy, image->getSize(this).cy);

		Ref< ui::Bitmap > newImage = new ui::Bitmap(width, height);
		newImage->copyImage(m_bitmapImages->getImage(this));
		newImage->copySubImage(image->getImage(this), Rect(Point(0, 0), image->getSize(this)), Point(m_bitmapImages->getSize(this).cx, 0));
		m_bitmapImages = newImage;
	}
	else
	{
		m_bitmapImages = image;
		m_imageWidth = std::max< uint32_t >(m_imageWidth, m_bitmapImages->getSize(this).cx / imageCount);
		m_imageHeight = std::max< uint32_t >(m_imageHeight, m_bitmapImages->getSize(this).cy);
	}

	return 0;
}

int32_t Tab::addPage(TabPage* page)
{
	if (page == 0)
		return -1;

	for (auto& ps : m_pages)
		ps.depth++;

	m_pages.push_back(PageState(page));

	if (m_selectedPage)
		m_selectedPage->setVisible(false);

	page->setRect(m_innerRect);
	page->setVisible(true);

	m_selectedPage = page;

	return int32_t(m_pages.size() - 1);
}

int32_t Tab::getPageCount() const
{
	return (int32_t)m_pages.size();
}

TabPage* Tab::getPage(int32_t index) const
{
	if (index >= 0 && index < (int32_t)m_pages.size())
		return m_pages[index].page;
	else
		return nullptr;
}

TabPage* Tab::getPageAt(const Point& position) const
{
	const auto rc = getInnerRect();
	if (!m_bottom && position.y >= rc.top + pixel(m_tabHeight))
		return nullptr;
	else if (m_bottom && position.y <= rc.bottom - pixel(m_tabHeight))
		return nullptr;

	for (const auto& ps : m_pages)
	{
		if (position.x <= ps.right)
			return ps.page;
	}

	return nullptr;
}

bool Tab::removePage(TabPage* page)
{
	auto it = std::find(m_pages.begin(), m_pages.end(), PageState(page));
	if (it == m_pages.end())
		return false;

	m_pages.erase(it);

	if (page == m_selectedPage)
	{
		page->setVisible(false);

		// Decrement all depths, to one found with zero depth is the new active page.
		for (auto& ps : m_pages)
			ps.depth--;

		PageState* newPageState = findPageState(0);
		if (newPageState)
		{
			m_selectedPage = newPageState->page;
			m_selectedPage->setVisible(true);
			m_selectedPage->setRect(m_innerRect);
		}
		else
			m_selectedPage = nullptr;
	}

	return true;
}

void Tab::removeAllPages()
{
	m_pages.clear();
}

void Tab::setActivePage(TabPage* page)
{
	if (page == m_selectedPage)
		return;

	if (m_selectedPage)
		m_selectedPage->setVisible(false);

	if ((m_selectedPage = page) != nullptr)
	{
		PageState* state = findPageState(page);
		T_ASSERT(state);

		int32_t depth = state->depth;
		for (auto& ps : m_pages)
		{
			if (ps.depth <= depth)
				ps.depth++;
		}

		state->depth = 0;

		m_selectedPage->setVisible(true);
		m_selectedPage->setRect(m_innerRect);
	}
}

TabPage* Tab::getActivePage()
{
	return m_selectedPage;
}

TabPage* Tab::cycleActivePage(bool forward)
{
	if (m_pages.size() < 2)
		return m_selectedPage;

	int32_t maxDepth = (int32_t)(m_pages.size() - 1);

	if (forward)
	{
		for (auto& ps : m_pages)
		{
			if (--ps.depth < 0)
				ps.depth = maxDepth;
		}
	}
	else
	{
		for (auto& ps : m_pages)
		{
			if (++ps.depth > maxDepth)
				ps.depth = 0;
		}
	}

	if (m_selectedPage)
		m_selectedPage->setVisible(false);

	PageState* pageState = findPageState(0);
	T_ASSERT(pageState);

	m_selectedPage = pageState->page;
	m_selectedPage->setVisible(true);
	m_selectedPage->setRect(m_innerRect);

	update();

	return m_selectedPage;
}

Size Tab::getMinimumSize() const
{
	return Size(256, 256);
}

Size Tab::getPreferredSize(const Size& hint) const
{
	return Size(256, 256);
}

void Tab::eventMouseTrack(MouseTrackEvent* event)
{
	if (!event->entered())
	{
		if (m_hoverPage)
		{
			m_hoverPage = nullptr;
			update();
		}
	}
}

void Tab::eventMouseMove(MouseMoveEvent* event)
{
	Point pnt = event->getPosition();
	Rect inner = Widget::getInnerRect();

	int32_t y0, y1;
	if (!m_bottom)
	{
		y0 = inner.top;
		y1 = inner.top + pixel(m_tabHeight);
	}
	else
	{
		y0 = inner.bottom - pixel(m_tabHeight);
		y1 = inner.bottom;
	}

	if (pnt.y >= y0 && pnt.y <= y1)
	{
		Ref< TabPage > hoverPage = getPageAt(pnt);
		if (hoverPage != m_hoverPage)
		{
			m_hoverPage = hoverPage;
			update();
		}
	}
}

void Tab::eventButtonDown(MouseButtonDownEvent* event)
{
	Point pnt = event->getPosition();
	Rect inner = Widget::getInnerRect();

	int32_t y0, y1;
	if (!m_bottom)
	{
		y0 = inner.top;
		y1 = inner.top + pixel(m_tabHeight);
	}
	else
	{
		y0 = inner.bottom - pixel(m_tabHeight);
		y1 = inner.bottom;
	}

	if (pnt.y >= y0 && pnt.y <= y1)
	{
		PageState* selectedPageState = nullptr;
		for (auto& ps : m_pages)
		{
			if (pnt.x <= ps.right)
			{
				selectedPageState = &ps;
				break;
			}
		}

		bool closed = false;
		if (m_closeButton && selectedPageState)
		{
			if (pnt.x >= selectedPageState->right - pixel(16_ut) && pnt.x < selectedPageState->right)
			{
				TabCloseEvent closeEvent(this, selectedPageState->page);
				raiseEvent(&closeEvent);
				closed = true;

				// User are allowed to destroy the tab when the last page is closed
				// thus we need to check if widget has been destroyed before continue.
				if (getIWidget() == nullptr)
					return;
			}
		}

		if (!closed && selectedPageState && selectedPageState->page != m_selectedPage)
		{
			if (m_selectedPage)
				m_selectedPage->setVisible(false);

			TabSelectionChangeEvent selectionChangeEvent(this, selectedPageState->page);
			raiseEvent(&selectionChangeEvent);

			if ((m_selectedPage = selectedPageState->page) != nullptr)
			{
				for (auto& ps : m_pages)
				{
					if (ps.depth < selectedPageState->depth)
						ps.depth++;
				}
				selectedPageState->depth = 0;
				m_selectedPage->setVisible(true);
				m_selectedPage->setRect(m_innerRect);
			}
		}

		setFocus();
		update();
	}
}

void Tab::eventSize(SizeEvent* event)
{
	m_innerRect = Widget::getInnerRect();
	if (!m_bottom)
		m_innerRect.top += pixel(m_tabHeight);
	else
		m_innerRect.bottom -= pixel(m_tabHeight);

	if (m_drawBorder)
		m_innerRect = m_innerRect.inflate(-1, -1);

	if (m_selectedPage)
		m_selectedPage->setRect(m_innerRect);

	update();

	event->consume();
}

void Tab::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcPaint = event->getUpdateRect();
	const Rect rcInner = Widget::getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	int32_t y0, y1;
	if (!m_bottom)
	{
		y0 = rcInner.top;
		y1 = rcInner.top + pixel(m_tabHeight);
	}
	else
	{
		y0 = rcInner.bottom - pixel(m_tabHeight);
		y1 = rcInner.bottom;
	}

	Rect rcTabs(rcInner.left, y0, rcInner.right, y1);

	// Fill tab background.
	canvas.setBackground(ss->getColor(this, isEnable(true) ? L"background-color" : L"background-color-disabled"));
	canvas.fillRect(rcTabs);

	// Draw tab pages.
	if (!m_pages.empty())
	{
		int32_t left = rcTabs.left;
		for (auto& ps : m_pages)
		{
			const TabPage* page = ps.page;
			const std::wstring text = page->getText();
			const Size sizText = canvas.getFontMetric().getExtent(text);

			int32_t tabWidthNoMargin = sizText.cx;
			if (m_bitmapImages != nullptr && page->getImageIndex() >= 0)
			{
				const Size bitmapSize = m_bitmapImages->getSize(this);
				tabWidthNoMargin += bitmapSize.cy;
			}
			if (m_closeButton)
			{
				const Size closeSize = m_bitmapClose->getSize(this);
				tabWidthNoMargin += closeSize.cx + pixel(4_ut);
			}

			const int32_t tabWidth = tabWidthNoMargin + pixel(12_ut);

			// Save right separator position in vector.
			ps.right = left + tabWidth;

			// Draw only those tabs that are visible.
			if (left < rcTabs.right)
			{
				// Calculate tab item rectangle.
				Rect rcTab(
					left,
					rcTabs.top,
					left + tabWidth,
					rcTabs.bottom
				);
				if (m_drawLine)
				{
					if (!m_bottom)
						rcTab.bottom -= 2;
					else
						rcTab.top += 2;
				}

				// Highlight selected tab.
				if (page == m_selectedPage)
				{
					canvas.setBackground(ss->getColor(this, L"tab-background-color"));
					canvas.fillRect(rcTab);
				}
				else if (page == m_hoverPage)
				{
					canvas.setBackground(ss->getColor(this, L"tab-background-color-hover"));
					canvas.fillRect(rcTab);
				}

				// Draw icon, offset text if icon is visible.
				int32_t textOffset = 0;
				if (m_bitmapImages != nullptr && page->getImageIndex() >= 0)
				{
					const Size bitmapSize = m_bitmapImages->getSize(this);
					canvas.drawBitmap(
						Point(left + pixel(4_ut), rcTab.getCenter().y - bitmapSize.cy / 2 + pixel(1_ut)),
						Point(page->getImageIndex() * bitmapSize.cy, 0),
						Size(bitmapSize.cy, bitmapSize.cy),
						m_bitmapImages,
						BlendMode::Alpha
					);
					textOffset += bitmapSize.cy;
				}

				// Draw close button.
				if (m_closeButton && (page == m_selectedPage || page == m_hoverPage))
				{
					const Size closeSize = m_bitmapClose->getSize(this);
					canvas.drawBitmap(
						Point(rcTab.right - closeSize.cx - pixel(4_ut), rcTab.getCenter().y - closeSize.cy / 2 + pixel(1_ut)),
						Point(0, 0),
						closeSize,
						m_bitmapClose,
						BlendMode::Alpha
					);
				}

				// Draw text.
				const Rect rcTabText(
					left + textOffset + pixel(4_ut),
					rcTab.top,
					left + textOffset + pixel(4_ut) + sizText.cx,
					rcTab.bottom
				);
				if (isEnable(true))
				{
					canvas.setForeground(ss->getColor(this, (page == m_selectedPage || page == m_hoverPage) ? L"tab-color-active" : L"tab-color-inactive"));
					canvas.drawText(rcTabText, text, AnLeft, AnCenter);
				}
				else
				{
					canvas.setForeground(ss->getColor(this, L"tab-color-disabled"));
					canvas.drawText(rcTabText, text, AnLeft, AnCenter);
				}
			}

			left += tabWidth;
		}
	}
	else
	{
		// No tab pages, fill solid background.
		const Rect rcTabItem(
			rcInner.left,
			m_bottom ? rcInner.top : rcInner.top + pixel(m_tabHeight),
			rcInner.right,
			m_bottom ? rcInner.bottom - pixel(m_tabHeight) : rcInner.bottom
		);
		canvas.setBackground(ss->getColor(this, isEnable(true) ? L"background-color" : L"background-color-disabled"));
		canvas.fillRect(rcTabItem);
	}

	// Draw line.
	if (m_drawLine)
	{
		canvas.setForeground(ss->getColor(this, L"tab-line-color"));
		if (!m_bottom)
		{
			canvas.drawLine(rcTabs.left, rcTabs.bottom - 2, rcTabs.right, rcTabs.bottom - 2);
			canvas.drawLine(rcTabs.left, rcTabs.bottom - 1, rcTabs.right, rcTabs.bottom - 1);
		}
		else
		{
			canvas.drawLine(rcTabs.left, rcTabs.top, rcTabs.right, rcTabs.top);
			canvas.drawLine(rcTabs.left, rcTabs.top + 1, rcTabs.right, rcTabs.top + 1);
		}
	}

	// Draw surrounding gray border.
	if (m_drawBorder)
	{
		canvas.setForeground(ss->getColor(this, L"border-color"));
		canvas.drawRect(rcInner);
	}

	event->consume();
}

Tab::PageState* Tab::findPageState(const TabPage* page)
{
	for (auto& ps : m_pages)
	{
		if (ps.page == page)
			return &ps;
	}
	return nullptr;
}

Tab::PageState* Tab::findPageState(int32_t depth)
{
	for (auto& ps : m_pages)
	{
		if (ps.depth == depth)
			return &ps;
	}
	return nullptr;
}

Tab::PageState::PageState(TabPage* page_)
:	page(page_)
{
}

bool Tab::PageState::operator == (const Tab::PageState& rh) const
{
	return page == rh.page;
}

}
