#include "Ui/PopupMenu.h"
#include "Ui/Widget.h"
#include "Ui/Application.h"
#include "Ui/Itf/IPopupMenu.h"
#include "Ui/Itf/IWidget.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PopupMenu", PopupMenu, EventSubject)

PopupMenu::PopupMenu()
:	m_popupMenu(0)
{
}

PopupMenu::~PopupMenu()
{
	T_ASSERT_M (!m_popupMenu, L"PopupMenu not destroyed");
}

bool PopupMenu::create()
{
	m_popupMenu = Application::getInstance().getWidgetFactory()->createPopupMenu(this);
	if (!m_popupMenu)
	{
		log::error << L"Failed to create native widget peer (PopupMenu)" << Endl;
		return false;
	}

	if (!m_popupMenu->create())
		return false;

	return true;
}

void PopupMenu::destroy()
{
	if (m_popupMenu)
	{
		m_popupMenu->destroy();
		m_popupMenu = 0;
	}
}

void PopupMenu::add(MenuItem* item)
{
	m_popupMenu->add(item);
}

MenuItem* PopupMenu::show(Widget* parent, const Point& at)
{
	if (!parent)
		return 0;

	return m_popupMenu->show(parent->getIWidget(), at);
}

	}
}
