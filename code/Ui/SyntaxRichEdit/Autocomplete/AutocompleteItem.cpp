/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/SyntaxRichEdit/Autocomplete/AutocompleteItem.h"
#include "Ui/Application.h"
#include "Ui/Auto/AutoWidget.h"
#include "Ui/Canvas.h"
#include "Ui/IBitmap.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.AutocompleteItem", AutocompleteItem, AutoWidgetCell)

AutocompleteItem::AutocompleteItem(const AutocompleteSuggestion& suggestion, IBitmap* icon)
:	m_suggestion(suggestion)
,	m_icon(icon)
{
}

void AutocompleteItem::setSelected(bool selected)
{
	m_selected = selected;
}

void AutocompleteItem::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = getStyleSheet();

	// Draw selection background
	if (m_selected)
	{
		canvas.setBackground(ss->getColor(this, L"background-color-selected"));
		canvas.fillRect(rect);
	}

	// Draw icon
	if (m_icon)
	{
		const Size iconSize = m_icon->getSize(getWidget());
		const int32_t iconY = rect.top + (rect.getHeight() - iconSize.cy) / 2;
		canvas.drawBitmap(Point(rect.left + 4, iconY), Point(0, 0), iconSize, m_icon, BlendMode::Alpha);
	}

	// Draw name
	canvas.setForeground(m_selected ? ss->getColor(this, L"color-selected") : ss->getColor(this, L"color"));
	canvas.drawText(Rect(rect.left + 24, rect.top + 2, rect.right - 4, rect.bottom - 2), m_suggestion.name, AnLeft, AnCenter);

	// Draw description (if room)
	if (!m_suggestion.description.empty() && rect.getWidth() > 300)
	{
		const FontMetric fm = canvas.getFontMetric();
		canvas.setForeground(m_selected ? ss->getColor(this, L"color-selected") : ss->getColor(this, L"color-description"));
		const int32_t nameWidth = fm.getExtent(m_suggestion.name).cx;
		canvas.drawText(Rect(rect.left + 24 + nameWidth + 10, rect.top + 2, rect.right - 4, rect.bottom - 2), L" - " + m_suggestion.description, AnLeft, AnCenter);
	}
}

}
