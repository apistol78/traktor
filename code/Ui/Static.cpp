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

Size Static::getPreferredSize(const Size& hint) const
{
	Size extent(0, 0);
	const auto fontMetric = getFontMetric();
	for (auto s : StringSplit< std::wstring >(getText(), L"\n\r"))
	{
		const auto sz = fontMetric.getExtent(s);
		extent.cx = std::max(sz.cx, extent.cx);
		extent.cy += sz.cy;
	}
	return extent + Size(pixel(1_ut), pixel(1_ut));
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

	Point pt(0, 0);

	const auto fontMetric = getFontMetric();
	for (auto s : StringSplit< std::wstring >(getText(), L"\n\r"))
	{
		const auto sz = fontMetric.getExtent(s);
		canvas.drawText(pt, s);
		pt.y += sz.cy;
	}

	event->consume();
}

}
