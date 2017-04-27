/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Wx/PopupMenuWx.h"
#include "Ui/Wx/EvtHandler.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/MenuItem.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace ui
	{

PopupMenuWx::PopupMenuWx(EventSubject* owner)
{
}

bool PopupMenuWx::create()
{
	T_CONNECT((&m_menu), wxEVT_COMMAND_MENU_SELECTED, wxCommandEvent, PopupMenuWx, &PopupMenuWx::onMenuSelected);
	return true;
}

void PopupMenuWx::destroy()
{
	delete this;
}

void PopupMenuWx::add(MenuItem* item)
{
	int id = int(m_flatten.size() + 1000);
	m_flatten.push_back(item);

	wxMenuItem* wx;
	if (item->count() > 0)
		wx = m_menu.Append(id, wstots(item->getText()).c_str(), buildSubMenu(item));
	else
	{
		if (item->getText() == L"-")
			wx = m_menu.AppendSeparator();
		else
			wx = m_menu.Append(id, wstots(item->getText()).c_str());
	}

	wx->Enable(item->isEnable());
}

MenuItem* PopupMenuWx::show(IWidget* parent, const Point& at)
{
	m_selected = 0;
	if (static_cast< wxWindow* >(parent->getInternalHandle())->PopupMenu(&m_menu))
		return m_selected;
	return 0;
}

wxMenu* PopupMenuWx::buildSubMenu(MenuItem* parentItem)
{
	wxMenu* menu = new wxMenu();
	T_CONNECT(menu, wxEVT_COMMAND_MENU_SELECTED, wxCommandEvent, PopupMenuWx, &PopupMenuWx::onMenuSelected);

	for (int i = 0; i < parentItem->count(); ++i)
	{
		Ref< MenuItem > item = parentItem->get(i);
		wxMenuItem* wx;

		if (item->getText() == L"-")
		{
			wx = menu->AppendSeparator();
		}
		else
		{
			int32_t id = int32_t(m_flatten.size() + 1000);
			m_flatten.push_back(item);

			if (item->count() > 0)
				wx = menu->Append(id, wstots(item->getText()).c_str(), buildSubMenu(item));
			else
				wx = menu->Append(id, wstots(item->getText()).c_str());
		}

		wx->Enable(item->isEnable());
	}

	return menu;
}

void PopupMenuWx::onMenuSelected(wxCommandEvent& event)
{
	int id = event.GetId();
	T_ASSERT (id >= 1000);
	m_selected = m_flatten[id - 1000];
}

	}
}
