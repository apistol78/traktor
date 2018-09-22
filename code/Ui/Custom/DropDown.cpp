/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Command.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/DropDown.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.DropDown", DropDown, Widget)

DropDown::DropDown()
:	m_selected(-1)
{
}

bool DropDown::create(Widget* parent, int style)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< MouseMoveEvent >(this, &DropDown::eventMouseMove);
	addEventHandler< MouseButtonDownEvent >(this, &DropDown::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &DropDown::eventButtonUp);
	addEventHandler< PaintEvent >(this, &DropDown::eventPaint);
	return true;
}

int32_t DropDown::add(const std::wstring& item, Object* data)
{
	m_items.push_back({ item, data });
	return int32_t(m_items.size() - 1);
}

bool DropDown::remove(int32_t index)
{
	if (index >= int32_t(m_items.size()))
		return false;

	auto i = m_items.begin() + index;
	m_items.erase(i);

	if (index >= m_selected)
		m_selected = -1;

	return true;
}

void DropDown::removeAll()
{
	m_items.resize(0);
	m_selected = -1;
}

int32_t DropDown::count() const
{
	return int32_t(m_items.size());
}

void DropDown::setItem(int32_t index, const std::wstring& item)
{
	m_items[index].text = item;
}

void DropDown::setData(int32_t index, Object* data)
{
	m_items[index].data = data;
}

std::wstring DropDown::getItem(int32_t index) const
{
	return (index >= 0 && index < int32_t(m_items.size())) ? m_items[index].text : L"";
}

Ref< Object > DropDown::getData(int32_t index) const
{
	return (index >= 0 && index < int32_t(m_items.size())) ? m_items[index].data : nullptr;
}

void DropDown::select(int32_t index)
{
	m_selected = index;
}

bool DropDown::select(const std::wstring& item)
{
	for (int32_t i = 0; i < count(); ++i)
	{
		if (getItem(i) == item)
		{
			select(i);
			return true;
		}
	}
	select(-1);
	return false;
}

int32_t DropDown::getSelected() const
{
	return m_selected;
}

std::wstring DropDown::getSelectedItem() const
{
	return getItem(m_selected);
}

Ref< Object > DropDown::getSelectedData() const
{
	return getData(m_selected);
}

Size DropDown::getPreferedSize() const
{
	const int32_t height = getFontMetric().getHeight() + dpi96(4) * 2;
	return Size(dpi96(200), height);
}

void DropDown::eventMouseMove(MouseMoveEvent* event)
{
	if (!isEnable())
		return;

	if (!hasCapture())
	{
		setCapture();
		update();
	}
	else if (!getInnerRect().inside(event->getPosition()))
	{
		releaseCapture();
		update();
	}
}

void DropDown::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable())
		return;

	if (m_items.empty())
		return;

	Menu menu;
	for (uint32_t i = 0; i < uint32_t(m_items.size()); ++i)
		menu.add(new MenuItem(Command(i), m_items[i].text));
	
	Rect rcInner = getInnerRect();
	const MenuItem* selectedItem = menu.showModal(this, rcInner.getBottomLeft(), rcInner.getWidth());
	if (selectedItem != nullptr && selectedItem->getCommand().getId() != m_selected)
	{
		m_selected = selectedItem->getCommand().getId();

		SelectionChangeEvent selectionChangeEvent(this);
		raiseEvent(&selectionChangeEvent);
	}

	update();
}

void DropDown::eventButtonUp(MouseButtonUpEvent* event)
{
	if (!isEnable())
		return;

	update();
}

void DropDown::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = event->getCanvas();

	Rect rcInner = getInnerRect();
	Point at = rcInner.getTopLeft();
	Size size = rcInner.getSize();
	int32_t sep = ui::dpi96(14);
	bool hover = isEnable() && hasCapture();

	Rect rcText(
		at.x + dpi96(4),
		at.y + 2,
		at.x + size.cx - sep - 2,
		at.y + size.cy - 2
	);
	Rect rcButton(
		at.x + size.cx - sep,
		at.y + 1,
		at.x + size.cx - 1,
		at.y + size.cy - 1
	);

	canvas.setBackground(ss->getColor(this, hover ? L"background-color-hover" : L"background-color"));
	canvas.fillRect(Rect(at, size));

	canvas.setBackground(ss->getColor(this, L"background-color-button"));
	canvas.fillRect(rcButton);

	if (hover)
	{
		canvas.setForeground(ss->getColor(this, L"color-hover"));
		canvas.drawRect(Rect(at, size));
		canvas.drawLine(rcButton.left - 1, rcButton.top, rcButton.left - 1, rcButton.bottom);
	}

	Point center = rcButton.getCenter();
	ui::Point pnts[] =
	{
		ui::Point(center.x - ui::dpi96(3), center.y - ui::dpi96(1)),
		ui::Point(center.x + ui::dpi96(2), center.y - ui::dpi96(1)),
		ui::Point(center.x - ui::dpi96(1), center.y + ui::dpi96(2))
	};

	canvas.setBackground(ss->getColor(this, L"color-arrow"));
	canvas.fillPolygon(pnts, 3);

	canvas.setForeground(ss->getColor(this, isEnable() ? L"color" : L"color-disabled"));
	canvas.drawText(rcText, getSelectedItem(), AnLeft, AnCenter);
}

		}
	}
}
