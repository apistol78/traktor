/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_MenuBarWx_H
#define traktor_ui_MenuBarWx_H

#include <vector>
#include <wx/wx.h>
#include "Core/RefArray.h"
#include "Ui/Itf/IMenuBar.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class MenuBarWx : public IMenuBar
{
public:
	MenuBarWx(EventSubject* owner);

	virtual bool create(IForm* form);

	virtual void destroy();

	virtual void add(MenuItem* item);

private:
	EventSubject* m_owner;
	wxFrame* m_frame;
	wxMenuBar* m_menuBar;
	RefArray< MenuItem > m_items;

	wxMenu* build(MenuItem* item);
	
	void onMenuSelected(wxCommandEvent& event);
};

	}
}

#endif	// traktor_ui_MenuBarWx_H
