#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Menu.h"
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

bool MenuShell::getItemRect(const MenuItem* item, Rect& outItemRect) const
{
	Rect rcInner = getInnerRect();
	Point itemTopLeft(1, 1);
	for (auto it : m_items)
	{
		Size itemSize(rcInner.getWidth() - 2, item->getSize(this).cy);
		if (it == item)
		{
			outItemRect = Rect(itemTopLeft, itemSize);
			return true;
		}
		itemTopLeft.y += itemSize.cy;
	}
	return false;
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
	MenuItem* item = getItem(event->getPosition());
	if (item != m_trackItem)
	{
		//if (hasCapture())
		//	releaseCapture();

		safeDestroy(m_trackSubMenu);

		if ((m_trackItem = item) != nullptr)
		{
			if (item->count() > 0)
			{
				Menu sm;

				// Build sub menu.
				for (int32_t i = 0; i < item->count(); ++i)
					sm.add(item->get(i));

				// Get position of clicked item.
				Rect rc;
				getItemRect(item, rc);

				// Show sub menu.
				m_trackSubMenu = sm.show(this, Point(rc.right, rc.top));
				T_FATAL_ASSERT (m_trackSubMenu != nullptr);

				m_trackSubMenu->addEventHandler< MenuClickEvent >([&](MenuClickEvent* e) {
					// Sub item selected, re-issue event in this shell.
					MenuClickEvent clickEvent(this, e->getItem(), e->getCommand());
					raiseEvent(&clickEvent);
				});
			}

			//setCapture();
		}

		update();
	}
}

void MenuShell::eventButtonDown(MouseButtonDownEvent* event)
{
	MenuItem* item = getItem(event->getPosition());
	if (item == nullptr)
	{
		MenuClickEvent clickEvent(this, nullptr, Command());
		raiseEvent(&clickEvent);
	}
}

void MenuShell::eventButtonUp(MouseButtonUpEvent* event)
{
	MenuItem* item = getItem(event->getPosition());
	if (item && item->isEnable())
	{
		MenuClickEvent clickEvent(this, item, item->getCommand());
		raiseEvent(&clickEvent);
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
		item->paint(this, canvas, Rect(itemTopLeft, itemSize), bool(item == m_trackItem));
		itemTopLeft.y += itemSize.cy;
	}

	canvas.setForeground(ss->getColor(this, L"border-color"));
	canvas.drawRect(rcInner);

	e->consume();
}

	}
}
