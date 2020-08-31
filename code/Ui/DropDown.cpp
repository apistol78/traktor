#include "Core/Io/StringOutputStream.h"
#include "Ui/Application.h"
#include "Ui/DropDown.h"
#include "Ui/Canvas.h"
#include "Ui/Command.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleSheet.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.DropDown", DropDown, Widget)

DropDown::DropDown()
:	m_multiple(false)
,	m_hover(false)
{
}

bool DropDown::create(Widget* parent, int32_t style)
{
	if (!Widget::create(parent, style))
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
	int32_t s = getSelected();
	if (s >= 0)
		return getItem(s);
	else
		return L"";
}

Object* DropDown::getSelectedData() const
{
	int32_t s = getSelected();
	if (s >= 0)
		return getData(s);
	else
		return nullptr;
}

Size DropDown::getPreferedSize() const
{
	const int32_t marginX = dpi96(16);
	const int32_t marginY = dpi96(4);

	FontMetric fm = getFontMetric();

	int32_t h = fm.getHeight();
	int32_t w = 0;
	for (const auto& item : m_items)
	{
		int32_t iw = fm.getExtent(item.text).cx;
		w = std::max(w, iw);
	}

	return Size(w + marginX * 2, h + marginY * 2);
}

void DropDown::eventMouseTrack(MouseTrackEvent* event)
{
	m_hover = event->entered();
	update();
}

void DropDown::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable())
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

	Rect rcInner = getInnerRect();

	const MenuItem* selectedItem = menu.showModal(this, rcInner.getBottomLeft(), rcInner.getWidth(), 8);
	if (!selectedItem)
		return;

	int32_t index = selectedItem->getCommand().getId();

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
	bool hover = isEnable() && m_hover;

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
}
