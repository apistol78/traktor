#include <sstream>
#include "Ui/Wx/MenuBarWx.h"
#include "Ui/Wx/EvtHandler.h"
#include "Ui/Itf/IForm.h"
#include "Ui/MenuItem.h"
#include "Ui/Widget.h"
#include "Ui/Events/CommandEvent.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace ui
	{

MenuBarWx::MenuBarWx(EventSubject* owner)
:	m_owner(owner)
,	m_frame(0)
,	m_menuBar(0)
{
}

bool MenuBarWx::create(IForm* form)
{
	m_frame = static_cast< wxFrame* >(form->getInternalHandle());

	m_menuBar = new wxMenuBar();
	m_frame->SetMenuBar(m_menuBar);

	T_CONNECT(m_frame, wxEVT_COMMAND_MENU_SELECTED, wxCommandEvent, MenuBarWx, &MenuBarWx::onMenuSelected);

	return true;
}

void MenuBarWx::destroy()
{
	if (!m_frame)
		return;

	m_frame->SetMenuBar(0);
	m_frame = 0;
	
	m_menuBar->Destroy();
	m_menuBar = 0;
}

void MenuBarWx::add(MenuItem* item)
{
	wxMenu* menu = build(item);
	T_ASSERT (menu);

	m_menuBar->Append(menu, wstots(item->getText()).c_str());

	m_frame->Refresh();
}

wxMenu* MenuBarWx::build(MenuItem* item)
{
	wxMenu* menu = new wxMenu();
	for (int i = 0; i < item->count(); ++i)
	{
		MenuItem* sub = item->get(i);
		
		if (sub->getText() == L"-")
		{
			wxMenuItem* wx = menu->AppendSeparator();
			wx->Enable(sub->isEnable());
		}
		else
		{
			wxMenu* subMenu = sub->count() ? build(sub) : 0;
			int menuId = int(m_items.size() + 1000);

			std::wstringstream ss;
			ss << sub->getText();

			// Create menu item.
			wxMenuItem* wx = menu->Append(menuId, wstots(ss.str()).c_str(), subMenu);
			wx->Enable(sub->isEnable());

			// Save reference to item.
			m_items.push_back(sub);
		}
	}
	return menu;
}

void MenuBarWx::onMenuSelected(wxCommandEvent& event)
{
	int id = event.GetId();
	T_ASSERT (id >= 1000);
	
	MenuClickEvent menuClickEvent(m_owner, m_items[id - 1000], m_items[id - 1000]->getCommand());
	m_owner->raiseEvent(&menuClickEvent);
}

	}
}
