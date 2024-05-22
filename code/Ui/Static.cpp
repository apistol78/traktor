/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/StringSplit.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Static.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Static", Static, Widget)

bool Static::create(Widget* parent, const std::wstring& text)
{
	if (!Widget::create(parent))
		return false;

	setText(text);
	addEventHandler< PaintEvent >(this, &Static::eventPaint);
	return true;
}

void Static::setText(const std::wstring& text)
{
	const auto fontMetric = getFontMetric();

	Widget::setText(text);

	// Split text into lines and also calculate bounding extent.
	m_extent = Size(0, 0);
	for (auto s : StringSplit< std::wstring >(text, L"\n\r"))
	{
		const auto sz = fontMetric.getExtent(s);
		m_lines.push_back({ m_extent.cy, s });
		m_extent.cx = std::max(sz.cx, m_extent.cx);
		m_extent.cy += sz.cy;
	}
}

Size Static::getPreferredSize(const Size& hint) const
{
	return m_extent + Size(pixel(1_ut), pixel(1_ut));
}

Size Static::getMaximumSize() const
{
	return Size(65535, 65535);
}

void Static::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	canvas.setForeground(ss->getColor(this, isEnable(true) ? L"color" : L"color-disabled"));
	for (const auto& line : m_lines)
		canvas.drawText(Point(0, line.first), line.second);

	event->consume();
}

}
