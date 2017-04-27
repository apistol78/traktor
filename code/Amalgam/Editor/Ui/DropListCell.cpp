/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Editor/HostEnumerator.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Amalgam/Editor/Ui/DropListCell.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Command.h"
#include "Ui/MenuItem.h"
#include "Ui/PopupMenu.h"
#include "Ui/Custom/Auto/AutoWidget.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.DropListCell", DropListCell, ui::custom::AutoWidgetCell)

DropListCell::DropListCell(HostEnumerator* hostEnumerator, TargetInstance* instance)
:	m_hostEnumerator(hostEnumerator)
,	m_instance(instance)
{
}

void DropListCell::mouseDown(ui::MouseButtonDownEvent* event, const ui::Point& position)
{
	ui::PopupMenu menu;
	if (menu.create())
	{
		std::wstring platformName = m_instance->getPlatformName();

		int32_t count = m_hostEnumerator->count();
		for (int32_t i = 0; i < count; ++i)
		{
			if (m_hostEnumerator->supportPlatform(i, platformName))
			{
				const std::wstring& description = m_hostEnumerator->getDescription(i);
				menu.add(new ui::MenuItem(ui::Command(i), description));
			}
		}

		Ref< ui::MenuItem > selectedItem = menu.show(getWidget< ui::custom::AutoWidget >(), m_menuPosition);
		if (selectedItem)
			m_instance->setDeployHostId(selectedItem->getCommand().getId());

		menu.destroy();
	}
}

void DropListCell::paint(ui::Canvas& canvas, const ui::Rect& rect)
{
	ui::Size size = rect.getSize();

	int32_t sep = ui::scaleBySystemDPI(14);

	ui::Rect rcText(
		rect.left + 4,
		rect.top + 1,
		rect.right - sep - 2,
		rect.bottom - 1
	);
	ui::Rect rcButton(
		rect.right - sep,
		rect.top,
		rect.right,
		rect.bottom
	);

	canvas.setBackground(ui::getSystemColor(ui::ScWindowBackground));
	canvas.fillRect(rect);

	canvas.setBackground(ui::getSystemColor(ui::ScMenuBackground));
	canvas.fillRect(rcButton);

	canvas.setForeground(Color4ub(128, 128, 140));
	canvas.drawRect(rect);
	canvas.drawLine(rcButton.left - 1, rcButton.top, rcButton.left - 1, rcButton.bottom - 1);

	ui::Point center = rcButton.getCenter();
	ui::Point pnts[] =
	{
		ui::Point(center.x - ui::scaleBySystemDPI(3), center.y - ui::scaleBySystemDPI(1)),
		ui::Point(center.x + ui::scaleBySystemDPI(2), center.y - ui::scaleBySystemDPI(1)),
		ui::Point(center.x - ui::scaleBySystemDPI(1), center.y + ui::scaleBySystemDPI(2))
	};

	canvas.setBackground(ui::getSystemColor(ui::ScWindowText));
	canvas.fillPolygon(pnts, 3);

	int32_t id = m_instance->getDeployHostId();
	if (id >= 0)
	{
		const std::wstring& description = m_hostEnumerator->getDescription(id);
		canvas.setForeground(ui::getSystemColor(ui::ScWindowText));
		canvas.drawText(rcText, description, ui::AnLeft, ui::AnCenter);
	}

	m_menuPosition = ui::Point(rect.left, rect.bottom);
}

	}
}
