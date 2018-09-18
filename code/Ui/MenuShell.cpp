#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/MenuItem.h"
#include "Ui/MenuShell.h"
#include "Ui/StyleSheet.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MenuShell", MenuShell, Widget)
	
bool MenuShell::create(Widget* parent)
{
	if (!Widget::create(parent, ui::WsDoubleBuffer))
		return false;

	addEventHandler< MouseMoveEvent >(this, &MenuShell::eventMouseMove);
	addEventHandler< MouseButtonDownEvent >(this, &MenuShell::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &MenuShell::eventButtonUp);
	addEventHandler< PaintEvent >(this, &MenuShell::eventPaint);

	return true;
}

void MenuShell::add(MenuItem* item)
{
	m_items.push_back(item);
}

MenuItem* MenuShell::getItem(const Point& at) const
{
	Rect rcInner = getInnerRect();
	Point itemTopLeft(1, 1);
	for (auto item : m_items)
	{
		Size itemSize(rcInner.getWidth() - 2, item->getSize(this).cy);
		if (Rect(itemTopLeft, itemSize).inside(at))
			return item;
		itemTopLeft.y += itemSize.cy;
	}
	return 0;
}

Size MenuShell::getMinimumSize() const
{
	Size minimumSize(0, 0);
	for (auto item : m_items)
	{
		Size itemSize = item->getSize(this);
		minimumSize.cx = std::max(minimumSize.cx, itemSize.cx);
		minimumSize.cy += itemSize.cy;
	}
	return minimumSize + Size(2, 2);
}

Size MenuShell::getPreferedSize() const
{
	return getMinimumSize();
}

void MenuShell::eventMouseMove(MouseMoveEvent* event)
{
	if (!isEnable())
		return;

		log::info << event->getPosition().x << L", " << event->getPosition().y << Endl;

	MenuItem* item = getItem(event->getPosition());
	if (item != m_trackItem)
	{
		if (m_trackItem)
		{
			m_trackItem->mouseLeave(this, event);
			m_trackItem = 0;
		}

		m_trackItem = item;

		if (item && item->mouseEnter(this, event))
		{
			//setCapture();
			m_trackItem = item;
		}
		// else
		// {
		// 	releaseCapture();
		// }

		update();
	}
}

void MenuShell::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable())
		return;

	MenuItem* item = getItem(event->getPosition());
	if (item && item->isEnable())
	{
		item->buttonDown(this, event);
		update();
	}
	else
	{
		MenuClickEvent clickEvent(this, nullptr, Command());
		raiseEvent(&clickEvent);
	}
}

void MenuShell::eventButtonUp(MouseButtonUpEvent* event)
{
	if (!isEnable())
		return;

	MenuItem* item = getItem(event->getPosition());
	if (item && item->isEnable())
	{
		item->buttonUp(this, event);
		update();
	}
}

void MenuShell::eventPaint(PaintEvent* e)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = e->getCanvas();
	Rect rcInner = getInnerRect();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	Point itemTopLeft(1, 1);
	for (auto item : m_items)
	{
		Size itemSize(rcInner.getWidth() - 2, item->getSize(this).cy);
		item->paint(this, canvas, Rect(itemTopLeft, itemSize));
		itemTopLeft.y += itemSize.cy;
	}

	canvas.setForeground(ss->getColor(this, L"border-color"));
	canvas.drawRect(rcInner);

	e->consume();
}

	}
}
