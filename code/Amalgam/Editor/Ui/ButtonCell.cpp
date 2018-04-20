/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Editor/Ui/ButtonCell.h"
#include "Ui/Application.h"
#include "Ui/Custom/Auto/AutoWidget.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ButtonCell", ButtonCell, ui::custom::AutoWidgetCell)

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
	ui::Size bitmapSize = m_bitmap->getSize();

	ui::Point position = rect.getTopLeft();
	position.y += (rect.getHeight() - bitmapSize.cy) / 2;

	if (m_down)
	{
		position.x += ui::dpi96(1);
		position.y += ui::dpi96(1);
	}

	canvas.drawBitmap(
		position,
		ui::Point(0, 0),
		bitmapSize,
		m_bitmap,
		ui::BmAlpha
	);
}

	}
}
