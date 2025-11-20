/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompletePopup.h"

#include "Ui/Application.h"
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompleteSelectEvent.h"
#include "Ui/Canvas.h"
#include "Ui/IBitmap.h"
#include "Ui/ScrollBar.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.AutocompletePopup", AutocompletePopup, Widget)

AutocompletePopup::AutocompletePopup()
{
}

bool AutocompletePopup::create(Widget* parent)
{
	if (!Widget::create(parent, WsNone))
		return false;

	addEventHandler< PaintEvent >(this, &AutocompletePopup::eventPaint);
	addEventHandler< SizeEvent >(this, &AutocompletePopup::eventSize);
	addEventHandler< MouseButtonDownEvent >(this, &AutocompletePopup::eventButtonDown);
	addEventHandler< MouseMoveEvent >(this, &AutocompletePopup::eventMouseMove);

	m_scrollBar = new ScrollBar();
	m_scrollBar->create(this, ScrollBar::WsVertical);
	m_scrollBar->hide();

	return true;
}

void AutocompletePopup::setSuggestions(IAutocompleteProvider* provider, const std::vector< AutocompleteSuggestion >& suggestions)
{
	m_provider = provider;
	m_suggestions = suggestions;
	m_selectedIndex = 0;
	m_scrollOffset = 0;
	updateScrollBar();
	update();
}

void AutocompletePopup::clearSuggestions()
{
	m_suggestions.clear();
	m_selectedIndex = 0;
	m_scrollOffset = 0;
	update();
}

int32_t AutocompletePopup::getSelectedIndex() const
{
	return m_selectedIndex;
}

void AutocompletePopup::setSelectedIndex(int32_t index)
{
	if (m_suggestions.empty())
		return;

	m_selectedIndex = std::max(0, std::min(index, (int32_t)m_suggestions.size() - 1));

	// Scroll to keep selection visible
	if (m_selectedIndex < m_scrollOffset)
		m_scrollOffset = m_selectedIndex;
	else if (m_selectedIndex >= m_scrollOffset + m_visibleItems)
		m_scrollOffset = m_selectedIndex - m_visibleItems + 1;

	updateScrollBar();
	update();
}

const AutocompleteSuggestion* AutocompletePopup::getSelectedSuggestion() const
{
	if (m_selectedIndex >= 0 && m_selectedIndex < (int32_t)m_suggestions.size())
		return &m_suggestions[m_selectedIndex];
	return nullptr;
}

bool AutocompletePopup::selectNext()
{
	if (m_suggestions.empty())
		return false;

	if (m_selectedIndex < (int32_t)m_suggestions.size() - 1)
	{
		setSelectedIndex(m_selectedIndex + 1);
		return true;
	}
	return false;
}

bool AutocompletePopup::selectPrevious()
{
	if (m_suggestions.empty())
		return false;

	if (m_selectedIndex > 0)
	{
		setSelectedIndex(m_selectedIndex - 1);
		return true;
	}
	return false;
}

int32_t AutocompletePopup::getSuggestionCount() const
{
	return (int32_t)m_suggestions.size();
}

void AutocompletePopup::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcClient = getInnerRect();
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	// Background
	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcClient);

	// Border
	canvas.setForeground(ss->getColor(this, L"border-color"));
	canvas.drawRect(rcClient);

	if (m_suggestions.empty())
		return;

	const FontMetric fm = getFontMetric();
	m_itemHeight = fm.getHeight() + 4;

	// Draw visible items
	int32_t y = rcClient.top + 2;
	const int32_t endIndex = std::min(m_scrollOffset + m_visibleItems, (int32_t)m_suggestions.size());

	for (int32_t i = m_scrollOffset; i < endIndex; ++i)
	{
		const auto& suggestion = m_suggestions[i];
		const bool selected = (i == m_selectedIndex);

		Rect itemRect(rcClient.left + 1, y, rcClient.right - 1, y + m_itemHeight);

		// Selection background
		if (selected)
		{
			canvas.setBackground(ss->getColor(this, L"background-color-selected"));
			canvas.fillRect(itemRect);
		}

		// Icon
		if (m_provider)
		{
			Ref< IBitmap > icon = m_provider->getSymbolIcon(suggestion.type);
			if (icon)
			{
				const Size iconSize = icon->getSize(this);
				const int32_t iconY = itemRect.top + (m_itemHeight - iconSize.cy) / 2;
				canvas.drawBitmap(Point(itemRect.left + 4, iconY), Point(0, 0), iconSize, icon, BlendMode::Alpha);
			}
		}

		// Text
		canvas.setForeground(selected ? ss->getColor(this, L"color-selected") : ss->getColor(this, L"color"));
		canvas.drawText(Rect(itemRect.left + 24, itemRect.top + 2, itemRect.right - 4, itemRect.bottom - 2), suggestion.name, AnLeft, AnCenter);

		// Description (if room)
		if (!suggestion.description.empty() && itemRect.getWidth() > 300)
		{
			canvas.setForeground(selected ? ss->getColor(this, L"color-selected") : ss->getColor(this, L"color-description"));
			const int32_t nameWidth = fm.getExtent(suggestion.name).cx;
			canvas.drawText(Rect(itemRect.left + 24 + nameWidth + 10, itemRect.top + 2, itemRect.right - 4, itemRect.bottom - 2), L" - " + suggestion.description, AnLeft, AnCenter);
		}

		y += m_itemHeight;
	}
}

void AutocompletePopup::eventSize(SizeEvent* event)
{
	const Rect rcClient = getInnerRect();
	m_visibleItems = std::max(1, (rcClient.getHeight() - 4) / m_itemHeight);
	updateScrollBar();
}

void AutocompletePopup::eventButtonDown(MouseButtonDownEvent* event)
{
	const int32_t index = hitTest(event->getPosition());
	if (index >= 0)
	{
		m_selectedIndex = index;
		update();

		// Trigger selection event when clicking with left button
		if (event->getButton() == MbtLeft)
		{
			AutocompleteSelectEvent selectEvent(this, m_suggestions[m_selectedIndex]);
			raiseEvent(&selectEvent);
		}

		event->consume();
	}
}

void AutocompletePopup::eventMouseMove(MouseMoveEvent* event)
{
	const int32_t index = hitTest(event->getPosition());
	if (index >= 0 && index != m_selectedIndex)
	{
		m_selectedIndex = index;
		update();
	}
}

void AutocompletePopup::updateScrollBar()
{
	if (m_suggestions.empty())
	{
		m_scrollBar->hide();
		return;
	}

	const int32_t totalItems = (int32_t)m_suggestions.size();
	if (totalItems > m_visibleItems)
	{
		const Rect rcClient = getInnerRect();
		m_scrollBar->setRect(Rect(rcClient.right - 16, rcClient.top, rcClient.right, rcClient.bottom));
		m_scrollBar->setRange(totalItems - m_visibleItems);
		m_scrollBar->setPage(m_visibleItems);
		m_scrollBar->setPosition(m_scrollOffset);
		m_scrollBar->show();
	}
	else
	{
		m_scrollBar->hide();
	}
}

int32_t AutocompletePopup::hitTest(const Point& pt) const
{
	const Rect rcClient = getInnerRect();
	if (!rcClient.inside(pt))
		return -1;

	const int32_t y = pt.y - rcClient.top - 2;
	const int32_t index = m_scrollOffset + (y / m_itemHeight);

	if (index >= 0 && index < (int32_t)m_suggestions.size())
		return index;

	return -1;
}

}
