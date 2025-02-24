/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/Ui/ButtonCell.h"
#include "Ui/Application.h"
#include "Ui/Auto/AutoWidget.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.ButtonCell", ButtonCell, ui::AutoWidgetCell)

ButtonCell::ButtonCell(
	ui::IBitmap* bitmap,
	const ui::Command& command
)
:	m_bitmap(bitmap)
,	m_command(command)
,	m_enable(true)
,	m_down(false)
{
}

void ButtonCell::setEnable(bool enable)
{
	m_enable = enable;
}

void ButtonCell::mouseDown(ui::MouseButtonDownEvent* event, const ui::Point& position)
{
	if (m_enable)
		m_down = true;
}

void ButtonCell::mouseUp(ui::MouseButtonUpEvent* event, const ui::Point& position)
{
	if (m_enable)
	{
		m_down = false;

		ui::ButtonClickEvent clickEvent(this, m_command);
		raiseEvent(&clickEvent);
	}
}

void ButtonCell::paint(ui::Canvas& canvas, const ui::Rect& rect)
{
	ui::Size bitmapSize = m_bitmap->getSize(getWidget());

	ui::Point position = rect.getTopLeft();
	position.y += (rect.getHeight() - bitmapSize.cy) / 2;

	if (m_down)
	{
		position.x += 1;
		position.y += 1;
	}

	canvas.drawBitmap(
		position,
		ui::Point(0, 0),
		bitmapSize,
		m_bitmap,
		ui::BlendMode::Alpha
	);
}

}
