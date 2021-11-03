#include "Runtime/Editor/HostEnumerator.h"
#include "Runtime/Editor/TargetInstance.h"
#include "Runtime/Editor/Ui/DropListCell.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Command.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleSheet.h"
#include "Ui/Auto/AutoWidget.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.DropListCell", DropListCell, ui::AutoWidgetCell)

DropListCell::DropListCell(HostEnumerator* hostEnumerator, TargetInstance* instance)
:	m_hostEnumerator(hostEnumerator)
,	m_instance(instance)
{
}

void DropListCell::mouseDown(ui::MouseButtonDownEvent* event, const ui::Point& position)
{
	std::wstring platformName = m_instance->getPlatformName();

	ui::Menu menu;
	int32_t count = m_hostEnumerator->count();
	for (int32_t i = 0; i < count; ++i)
	{
		if (m_hostEnumerator->supportPlatform(i, platformName))
		{
			const std::wstring& description = m_hostEnumerator->getDescription(i);
			menu.add(new ui::MenuItem(ui::Command(i), description));
		}
	}

	ui::Rect rcInner = getClientRect();
	const ui::MenuItem* selectedItem = menu.showModal(getWidget< ui::AutoWidget >(), rcInner.getBottomLeft(), rcInner.getWidth(), 8);
	if (selectedItem)
		m_instance->setDeployHostId(selectedItem->getCommand().getId());
}

void DropListCell::paint(ui::Canvas& canvas, const ui::Rect& rect)
{
	const ui::StyleSheet* ss = getWidget()->getStyleSheet();
	const ui::Rect& rcInner = rect;
	ui::Point at = rcInner.getTopLeft();
	ui::Size size = rcInner.getSize();
	int32_t sep = ui::dpi96(14);
	bool hover = false; //isEnable() && hasCapture();

	ui::Rect rcText(
		at.x + ui::dpi96(4),
		at.y + 2,
		at.x + size.cx - sep - 2,
		at.y + size.cy - 2
	);
	ui::Rect rcButton(
		at.x + size.cx - sep,
		at.y + 1,
		at.x + size.cx - 1,
		at.y + size.cy - 1
	);

	canvas.setBackground(ss->getColor(this, hover ? L"background-color-hover" : L"background-color"));
	canvas.fillRect(rcInner);

	canvas.setBackground(ss->getColor(this, L"background-color-button"));
	canvas.fillRect(rcButton);

	if (hover)
	{
		canvas.setForeground(ss->getColor(this, L"color-hover"));
		canvas.drawRect(rcInner);
		canvas.drawLine(rcButton.left - 1, rcButton.top, rcButton.left - 1, rcButton.bottom);
	}

	ui::Point center = rcButton.getCenter();
	ui::Point pnts[] =
	{
		ui::Point(center.x - ui::dpi96(3), center.y - ui::dpi96(1)),
		ui::Point(center.x + ui::dpi96(2), center.y - ui::dpi96(1)),
		ui::Point(center.x - ui::dpi96(1), center.y + ui::dpi96(2))
	};

	canvas.setBackground(ss->getColor(this, L"color-arrow"));
	canvas.fillPolygon(pnts, 3);

	int32_t id = m_instance->getDeployHostId();
	if (id >= 0)
	{
		const std::wstring& description = m_hostEnumerator->getDescription(id);
		canvas.setForeground(ss->getColor(this, /*isEnable() ?*/ L"color" /*: L"color-disabled"*/));
		canvas.setClipRect(rcText);
		canvas.drawText(rcText, description, ui::AnLeft, ui::AnCenter);
		canvas.resetClipRect();
	}
}

	}
}
