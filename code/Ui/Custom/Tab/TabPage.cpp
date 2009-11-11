#include "Ui/Custom/Tab/TabPage.h"
#include "Ui/Custom/Tab/Tab.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/PaintEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.TabPage", TabPage, Container)

bool TabPage::create(Tab* tab, const std::wstring& text, Layout* layout)
{
	if (!Container::create(tab, WsNone, layout))
		return false;

	addPaintEventHandler(createMethodHandler(this, &TabPage::eventPaint));

	m_tab = tab;

	setText(text);

	return true;
}

void TabPage::setActive()
{
	m_tab->setActivePage(this);
}

bool TabPage::isActive() const
{
	return bool(m_tab->getActivePage() == this);
}

Ref< Tab > TabPage::getTab()
{
	return m_tab;
}

void TabPage::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent* >(event);
	
	Canvas& canvas = paintEvent->getCanvas();
	Rect rcInner = Widget::getInnerRect();

	canvas.setBackground(Color(224, 224, 224));
	canvas.setForeground(Color(196, 194, 195));
	canvas.fillGradientRect(rcInner, true);

	event->consume();
}

		}
	}
}
