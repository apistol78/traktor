/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarSeparator.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_separatorWidth = 11_ut;
const Unit c_separatorInset = 4_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarSeparator", ToolBarSeparator, ToolBarItem)

bool ToolBarSeparator::getToolTip(std::wstring& outToolTip) const
{
	return false;
}

Size ToolBarSeparator::getSize(const ToolBar* toolBar) const
{
	const Size imageSize = toolBar->getImageSize();
	return Size(toolBar->pixel(c_separatorWidth), imageSize.cy);
}

void ToolBarSeparator::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, const RefArray< IBitmap >& images)
{
	const StyleSheet* ss = toolBar->getStyleSheet();
	const Size size = getSize(toolBar);
	const int32_t inset = toolBar->pixel(c_separatorInset);
	const int32_t x = at.x + size.cx / 2;

	canvas.setForeground(ss->getColor(toolBar, L"item-color-seperator"));
	canvas.drawLine(x, at.y + inset, x, at.y + size.cy - inset);
}

bool ToolBarSeparator::mouseEnter(ToolBar* toolBar)
{
	return false;
}

void ToolBarSeparator::mouseLeave(ToolBar* toolBar)
{
}

void ToolBarSeparator::buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent)
{
}

void ToolBarSeparator::buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent)
{
}

}
