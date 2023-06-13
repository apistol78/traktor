/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

namespace traktor
{
	namespace ui
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
	Widget::setText(text);

	// Calculate prefered size from new text.
	Size extent(0, 0);
	auto fontMetric = getFontMetric();
	for (auto s : StringSplit< std::wstring >(getText(), L"\n\r"))
	{
		auto sz = fontMetric.getExtent(s);
		extent.cx = std::max(sz.cx, extent.cx);
		extent.cy += sz.cy;
	}
	m_preferedSize = extent + Size(pixel(1_ut), pixel(1_ut));
}

Size Static::getPreferredSize(const Size& hint) const
{
	return m_preferedSize;
}

Size Static::getMaximumSize() const
{
	return Size::max();
}

void Static::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	canvas.setForeground(ss->getColor(this, L"color"));

	Point pt(0, 0);

	auto fontMetric = getFontMetric();
	for (auto s : StringSplit< std::wstring >(getText(), L"\n\r"))
	{
		auto sz = fontMetric.getExtent(s);
		canvas.drawText(pt, s);
		pt.y += sz.cy;
	}

	event->consume();
}

	}
}
