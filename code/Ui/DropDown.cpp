/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Ui/Application.h"
#include "Ui/DropDown.h"
#include "Ui/Canvas.h"
#include "Ui/Command.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleConstants.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_fieldPad = 8_ut;
const Unit c_fieldPadY = 6_ut;
const Unit c_chevronArea = 18_ut;
const Unit c_chevronHalf = 3_ut;

void paintChevron(Widget* widget, Canvas& canvas, const Point& center)
{
	const int32_t h = widget->pixel(c_chevronHalf);
	canvas.setPenThickness(widget->pixel(2_ut));
	canvas.drawLine(center.x - h, center.y - h / 2, center.x, center.y + h - h / 2);
	canvas.drawLine(center.x, center.y + h - h / 2, center.x + h, center.y - h / 2);
	canvas.setPenThickness(1);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.DropDown", DropDown, Widget)

bool DropDown::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style | WsFocus))
		return false;

	m_multiple = (bool)((style & WsMultiple) != 0);

	addEventHandler< MouseTrackEvent >(this, &DropDown::eventMouseTrack);
	addEventHandler< MouseButtonDownEvent >(this, &DropDown::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &DropDown::eventButtonUp);
	addEventHandler< PaintEvent >(this, &DropDown::eventPaint);

	return true;
}

int32_t DropDown::add(const std::wstring& item, Object* data)
{
	m_items.push_back({ item, data, false });
	return (int32_t)m_items.size() - 1;
}

bool DropDown::remove(int32_t index)
{
	if (index >= (int32_t)m_items.size())
		return false;

	auto i = m_items.begin() + index;
	m_items.erase(i);
	return true;
}

void DropDown::removeAll()
{
	m_items.resize(0);
}

int32_t DropDown::count() const
{
	return (int32_t)m_items.size();
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
	return (index >= 0 && index < (int32_t)m_items.size()) ? m_items[index].text : L"";
}

Object* DropDown::getData(int32_t index) const
{
	return (index >= 0 && index < (int32_t)m_items.size()) ? m_items[index].data : nullptr;
}

void DropDown::select(int32_t index)
{
	if (!m_multiple)
	{
		for (auto& item : m_items)
			item.selected = false;
	}
	if (index >= 0)
		m_items[index].selected = true;
	update(nullptr, false);
}

void DropDown::unselect(int32_t index)
{
	if (index >= 0)
		m_items[index].selected = false;
	update(nullptr, false);
}

bool DropDown::selected(int32_t index) const
{
	return m_items[index].selected;
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
	for (int32_t i = 0; i < count(); ++i)
	{
		if (selected(i))
			return i;
	}
	return -1;
}

int32_t DropDown::getSelected(std::vector< int32_t >& selected) const
{
	selected.resize(0);
	for (int32_t i = 0; i < count(); ++i)
	{
		if (this->selected(i))
			selected.push_back(i);
	}
	return (int32_t)selected.size();
}

std::wstring DropDown::getSelectedItem() const
{
	const int32_t s = getSelected();
	if (s >= 0)
		return getItem(s);
	else
		return L"";
}

Object* DropDown::getSelectedData() const
{
	const int32_t s = getSelected();
	if (s >= 0)
		return getData(s);
	else
		return nullptr;
}

Size DropDown::getPreferredSize(const Size& hint) const
{
	const FontMetric fm = getFontMetric();

	int32_t w = 0;
	for (const auto& item : m_items)
		w = std::max(w, fm.getExtent(item.text).cx);

	return Size(
		w + pixel(c_fieldPad) * 2 + pixel(c_chevronArea),
		fm.getHeight() + pixel(c_fieldPadY) * 2
	);
}

Size DropDown::getMaximumSize() const
{
	const Size preferredSize = getPreferredSize(Size(0, 0));
	return Size(65535, preferredSize.cy);
}

void DropDown::eventMouseTrack(MouseTrackEvent* event)
{
	m_hover = event->entered();
	update();
}

void DropDown::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable(true))
		return;

	if (m_items.empty())
		return;

	Menu menu;
	for (uint32_t i = 0; i < uint32_t(m_items.size()); ++i)
	{
		if (!m_multiple)
			menu.add(new MenuItem(Command(i), m_items[i].text, false, nullptr));
		else
		{
			Ref< MenuItem > mi = new MenuItem(Command(i), m_items[i].text, true, nullptr);
			mi->setChecked(m_items[i].selected);
			menu.add(mi);
		}
	}

	const Rect rcInner = getInnerRect();

	const MenuItem* selectedItem = menu.showModal(this, rcInner.getBottomLeft(), rcInner.getWidth(), 8);
	if (!selectedItem)
		return;

	const int32_t index = selectedItem->getCommand().getId();

	if (!m_multiple)
	{
		if (index != getSelected())
		{
			select(index);

			SelectionChangeEvent selectionChangeEvent(this);
			raiseEvent(&selectionChangeEvent);
		}
	}
	else
	{
		if (selected(index))
			unselect(index);
		else
			select(index);

		SelectionChangeEvent selectionChangeEvent(this);
		raiseEvent(&selectionChangeEvent);
	}

	if (m_widget)
		update();
}

void DropDown::eventButtonUp(MouseButtonUpEvent* event)
{
	if (!isEnable(true))
		return;

	update();
}

void DropDown::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const StyleSheet* ss = getStyleSheet();

	const Rect rcInner = getInnerRect();
	const Point at = rcInner.getTopLeft();
	const Size size = rcInner.getSize();
	const int32_t radius = pixel(c_controlRadius);
	const int32_t chevron = pixel(c_chevronArea);
	const int32_t pad = pixel(c_fieldPad);
	const bool enabled = isEnable(true);
	const bool hover = enabled && m_hover;

	const Rect rcField(at, size);
	const Rect rcText(
		at.x + pad,
		at.y,
		at.x + size.cx - chevron,
		at.y + size.cy
	);

	// Cover the whole client area first; the rounded field leaves the four
	// corners of the rectangle uncovered.
	canvas.setBackground(ss->getColor(getParent(), L"background-color"));
	canvas.fillRect(rcInner);

	canvas.setBackground(ss->getColor(this, hover ? L"background-color-hover" : L"background-color"));
	canvas.fillRoundRect(rcField, radius);

	canvas.setForeground(ss->getColor(this, hover ? L"color-hover" : L"border-color"));
	canvas.drawRoundRect(rcField, radius);

	canvas.setForeground(ss->getColor(this, L"color-arrow"));
	paintChevron(this, canvas, Point(at.x + size.cx - chevron / 2, at.y + size.cy / 2));

	canvas.setForeground(ss->getColor(this, enabled ? L"color" : L"color-disabled"));

	if (!m_multiple)
		canvas.drawText(rcText, getSelectedItem(), AnLeft, AnCenter);
	else
	{
		StringOutputStream ss;
		for (const auto& item : m_items)
		{
			if (item.selected)
			{
				if (!ss.empty())
					ss << L", " << item.text;
				else
					ss << item.text;
			}
		}
		canvas.drawText(rcText, ss.str(), AnLeft, AnCenter);
	}
}

}
