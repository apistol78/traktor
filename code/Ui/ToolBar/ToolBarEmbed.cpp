/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/MathUtils.h"
#include "Ui/Widget.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarEmbed.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarEmbed", ToolBarEmbed, ToolBarItem)

ToolBarEmbed::ToolBarEmbed(Widget* widget, Unit width, Unit preferredHeight)
:	m_widget(widget)
,	m_width(width)
,	m_preferredHeight(preferredHeight)
{
}

bool ToolBarEmbed::getToolTip(std::wstring& outToolTip) const
{
	return false;
}

Size ToolBarEmbed::getSize(const ToolBar* toolBar) const
{
	const Size imageSize = toolBar->getImageSize();
	const Size preferedSize = m_widget->getPreferredSize(Size(toolBar->pixel(m_width), imageSize.cy));
	return Size(
		max(preferedSize.cx, toolBar->pixel(m_width)),
		m_preferredHeight <= 0_ut ? max(preferedSize.cy, imageSize.cy) : toolBar->pixel(m_preferredHeight)
	);
}

void ToolBarEmbed::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, const RefArray< IBitmap >& images)
{
	const Rect rc(at, getSize(toolBar));
	m_widget->setRect(rc);
}

bool ToolBarEmbed::mouseEnter(ToolBar* toolBar)
{
	return false;
}

void ToolBarEmbed::mouseLeave(ToolBar* toolBar)
{
}

void ToolBarEmbed::buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent)
{
}

void ToolBarEmbed::buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent)
{
}

}
